#!/usr/bin/env bash
#
# Parallel simulation runner (GNU parallel or xargs fallback)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GEANT4_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
PROJECT_ROOT="$(cd "${GEANT4_ROOT}/.." && pwd)"
CORSIKA_DIR="${PROJECT_ROOT}/corsika/outputs"

NUM_JOBS="${1:-}"
PARTICLE_TYPE="${2:-}"
SELECTION="${3:-all}"
NUM_EVENTS="${4:-}"

# Smart handling: if particle="all" and 3rd arg is a number, treat it as num_events
if [[ "${PARTICLE_TYPE}" == "all" ]] && [[ "${SELECTION}" =~ ^[0-9]+$ ]]; then
    NUM_EVENTS="${SELECTION}"
    SELECTION="all"
fi

print_usage() {
    cat << EOF
Usage: run_parallel.sh <jobs> <particle> <selection> [num_events]

Arguments:
  jobs:       Number of parallel jobs (recommended: 2-8)
  particle:   'gammas', 'protons', or 'all'
  selection:  Index spec (default: all)
  num_events: Optional: limit events per simulation (default: all)

Examples:
  run_parallel.sh 4 gammas all         # all gammas with 4 jobs
  run_parallel.sh 8 protons 1-10 100   # protons 1-10, 100 events each
  run_parallel.sh 2 gammas 1,5,10 50   # specific gammas, 50 events each
  run_parallel.sh 4 all                # all simulations with 4 jobs

Selection formats (same as run_simulations.sh):
  N           Single index
  N,M,K       Comma separated
  N-M         Range
  all         All files for that particle

EOF
}

if [[ -z "${NUM_JOBS}" ]] || [[ -z "${PARTICLE_TYPE}" ]]; then
    print_usage
    exit 1
fi

if ! [[ "${NUM_JOBS}" =~ ^[0-9]+$ ]] || (( NUM_JOBS < 1 )); then
    echo "ERROR: jobs must be a positive integer" >&2
    exit 1
fi

if [[ -n "${NUM_EVENTS}" ]] && ! [[ "${NUM_EVENTS}" =~ ^[0-9]+$ ]]; then
    echo "ERROR: num_events must be a positive integer" >&2
    exit 1
fi

# Build task list
TASK_LIST=$(mktemp)
trap "rm -f ${TASK_LIST}" EXIT

add_tasks() {
    local particle=$1
    local selection=$2

    if [[ "${selection}" == "all" ]]; then
        for dat_file in "${CORSIKA_DIR}/${particle}"/DAT[0-9]*; do
            [[ -f "${dat_file}" ]] || continue
            [[ "${dat_file}" =~ \.(log|long)$ ]] && continue

            local dat_number=$(basename "${dat_file}" | sed 's/DAT//')
            local index
            if [[ "${particle}" == "gammas" ]]; then
                index=$((10#${dat_number}))
            else
                index=$((10#${dat_number} - 30))
            fi
            echo "${particle} ${index}" >> "${TASK_LIST}"
        done
    elif [[ "${selection}" =~ ^[0-9]+-[0-9]+$ ]]; then
        local start=${selection%-*}
        local end=${selection#*-}
        for i in $(seq "${start}" "${end}"); do
            echo "${particle} ${i}" >> "${TASK_LIST}"
        done
    elif [[ "${selection}" =~ , ]]; then
        IFS=',' read -ra indices <<< "${selection}"
        for i in "${indices[@]}"; do
            echo "${particle} ${i}" >> "${TASK_LIST}"
        done
    else
        echo "${particle} ${selection}" >> "${TASK_LIST}"
    fi
}

if [[ "${PARTICLE_TYPE}" == "all" ]]; then
    echo "Building task list for all simulations..."
    add_tasks "gammas" "all"
    add_tasks "protons" "all"
else
    echo "Building task list for ${PARTICLE_TYPE}..."
    add_tasks "${PARTICLE_TYPE}" "${SELECTION}"
fi

TOTAL_TASKS=$(wc -l < "${TASK_LIST}")
if (( TOTAL_TASKS == 0 )); then
    echo "ERROR: No tasks to run" >&2
    exit 1
fi

echo "========================================================================"
echo "Parallel Simulation Run"
echo "========================================================================"
echo "Particle type:  ${PARTICLE_TYPE}"
echo "Selection:      ${SELECTION}"
echo "Total tasks:    ${TOTAL_TASKS}"
echo "Parallel jobs:  ${NUM_JOBS}"
echo "Num events:     ${NUM_EVENTS:-all}"
echo "========================================================================"

start_time=$(date +%s)

if command -v parallel &>/dev/null; then
    echo "Using GNU parallel"
    if [[ -n "${NUM_EVENTS}" ]]; then
        parallel -j "${NUM_JOBS}" --colsep ' ' \
            "${SCRIPT_DIR}/run_simulations.sh {1} {2} ${NUM_EVENTS}" :::: "${TASK_LIST}"
    else
        parallel -j "${NUM_JOBS}" --colsep ' ' \
            "${SCRIPT_DIR}/run_simulations.sh {1} {2}" :::: "${TASK_LIST}"
    fi
    EXIT_CODE=$?
elif command -v xargs &>/dev/null; then
    echo "Using xargs (install GNU parallel for better output)"
    if [[ -n "${NUM_EVENTS}" ]]; then
        xargs -n 2 -P "${NUM_JOBS}" -a "${TASK_LIST}" \
            bash -c "${SCRIPT_DIR}/run_simulations.sh \"\$1\" \"\$2\" ${NUM_EVENTS}" _
    else
        xargs -n 2 -P "${NUM_JOBS}" -a "${TASK_LIST}" \
            bash -c "${SCRIPT_DIR}/run_simulations.sh \"\$1\" \"\$2\"" _
    fi
    EXIT_CODE=$?
else
    echo "ERROR: Neither GNU parallel nor xargs available" >&2
    exit 1
fi

end_time=$(date +%s)
elapsed=$((end_time - start_time))

echo ""
echo "========================================================================"
echo "PARALLEL RUN COMPLETE"
echo "========================================================================"
echo "Total tasks:    ${TOTAL_TASKS}"
echo "Parallel jobs:  ${NUM_JOBS}"
echo "Total time:     ${elapsed} seconds"
echo "Avg time/task:  $((elapsed / TOTAL_TASKS)) seconds"
echo "========================================================================"

# Count successes and failures by checking output directories
success_count=$(find "${GEANT4_ROOT}/outputs" -name "detector_hits.txt" -newer "${TASK_LIST}" 2>/dev/null | wc -l)
echo "Successfully completed: ${success_count}/${TOTAL_TASKS}"

if [[ ${success_count} -lt ${TOTAL_TASKS} ]]; then
    failed=$((TOTAL_TASKS - success_count))
    echo "⚠ ${failed} simulation(s) may have failed"
    echo "Check individual log files in geant4/outputs/single_*/"
    exit 1
fi

echo "✓ All simulations completed successfully"
exit 0
