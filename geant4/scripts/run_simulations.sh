#!/usr/bin/env bash
#
# Main simulation runner - serial execution
#
# Examples:
#   run_simulations.sh gammas 1              # Single gamma simulation #1
#   run_simulations.sh protons 5             # Single proton simulation #5
#   run_simulations.sh gammas 1,5,10         # Multiple specific indices
#   run_simulations.sh protons 1-5           # Range of indices
#   run_simulations.sh gammas all            # All gamma simulations
#   run_simulations.sh protons all           # All proton simulations
#   run_simulations.sh all                   # Everything (60 simulations)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GEANT4_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
PROJECT_ROOT="$(cd "${GEANT4_ROOT}/.." && pwd)"
BUILD_DIR="${GEANT4_ROOT}/build"
CORSIKA_DIR="${PROJECT_ROOT}/corsika/outputs"
SRC_DIR="${PROJECT_ROOT}/src"
OUTPUTS_BASE="${GEANT4_ROOT}/outputs"

# ============================================================================
# Geant4 Environment
# ============================================================================

setup_geant4_env() {
    local g4_data=""

    # Try conda env locations
    if [[ -n "${CONDA_PREFIX:-}" ]]; then
        for subdir in share/geant4/data share/Geant4/data share/geant4-data; do
            if [[ -d "${CONDA_PREFIX}/${subdir}" ]]; then
                g4_data="${CONDA_PREFIX}/${subdir}"
                break
            fi
        done
    fi

    # Try system locations
    if [[ -z "${g4_data}" ]]; then
        for dir in /usr/local/share/Geant4*/data /usr/share/Geant4*/data /opt/geant4*/share/Geant4*/data; do
            if [[ -d "${dir}" ]]; then
                g4_data="${dir}"
                break
            fi
        done
    fi

    # Fallback: best-effort locate
    if [[ -z "${g4_data}" ]] && command -v locate &>/dev/null; then
        g4_data=$(locate -l 1 ENSDFSTATE 2>/dev/null | xargs -r dirname 2>/dev/null || echo "")
    fi

    if [[ -z "${g4_data}" ]] || [[ ! -d "${g4_data}" ]]; then
        echo "WARNING: Geant4 data not found. Simulations may fail." >&2
        echo "  Try: conda install -c conda-forge geant4-data" >&2
        return 0
    fi

    # Helper: pick first matching subdir by prefix
    pick_data_dir() {
        local base=$1
        local pattern=$2
        local match
        for match in "${base}"/${pattern}*/; do
            [[ -d "${match}" ]] && { echo "${match%/}"; return; }
        done
        echo ""
    }

    export G4ENSDFSTATEDATA=$(pick_data_dir "${g4_data}" "ENSDFSTATE")
    export G4LEVELGAMMADATA=$(pick_data_dir "${g4_data}" "PhotonEvaporation")
    export G4RADIOACTIVEDATA=$(pick_data_dir "${g4_data}" "RadioactiveDecay")
    export G4NEUTRONHPDATA=$(pick_data_dir "${g4_data}" "NDL")
    export G4LEDATA=$(pick_data_dir "${g4_data}" "EMLOW")
    export G4PIIDATA=$(pick_data_dir "${g4_data}" "PII")
    export G4SAIDXSDATA=$(pick_data_dir "${g4_data}" "SAIDDATA")
    export G4ABLADATA=$(pick_data_dir "${g4_data}" "ABLA")
    export G4INCLDATA=$(pick_data_dir "${g4_data}" "INCL")
    export G4REALSURFACEDATA=$(pick_data_dir "${g4_data}" "RealSurface")
    export G4PARTICLEXSDATA=$(pick_data_dir "${g4_data}" "PARTICLEXS")

    echo "Geant4 data directory: ${g4_data}" >&2
    echo "  G4ENSDFSTATEDATA=${G4ENSDFSTATEDATA}" >&2
    echo "  G4NEUTRONHPDATA=${G4NEUTRONHPDATA}" >&2
}

# ============================================================================
# Validation
# ============================================================================

if [[ ! -f "${BUILD_DIR}/condorSim" ]]; then
    echo "ERROR: condorSim executable not found in ${BUILD_DIR}" >&2
    echo "Please rebuild: cd ${BUILD_DIR} && cmake .. && make -j4" >&2
    exit 1
fi

if [[ ! -f "${SRC_DIR}/corsika_to_geant4.py" ]]; then
    echo "ERROR: corsika_to_geant4.py not found in ${SRC_DIR}" >&2
    exit 1
fi

setup_geant4_env

# ============================================================================
# Helpers
# ============================================================================

print_usage() {
    cat << EOF
Usage: run_simulations.sh <particle> <selection> [num_events]

Arguments:
  particle:    'gammas', 'protons', or 'all'
  selection:   Index/range spec (required unless particle='all')
  num_events:  Optional: limit events per simulation (default: all)

Selection formats:
  N           Single index (e.g., 1, 5, 10)
  N,M,K       Multiple indices (e.g., 1,5,10)
  N-M         Range (e.g., 1-5)
  all         All available files for that particle type

Index mapping:
  Gammas:  1-30  → DAT000001-DAT000030
  Protons: 1-30  → DAT000031-DAT000060

Examples:
  run_simulations.sh gammas 1              # gamma #1 (all events)
  run_simulations.sh protons 5 100         # proton #5 (first 100 events)
  run_simulations.sh gammas 1,5,10         # gammas 1, 5, 10
  run_simulations.sh protons 1-5 500       # protons 1-5 (500 events each)
  run_simulations.sh gammas all            # all 30 gamma files
  run_simulations.sh all                   # all 60 simulations

EOF
}

index_to_dat() {
    local particle=$1
    local index=$2
    if [[ "${particle}" == "gammas" ]]; then
        printf "%06d" "${index}"
    else
        printf "%06d" $((index + 30))
    fi
}

parse_selection() {
    local selection=$1
    local indices=()

    if [[ "${selection}" == "all" ]]; then
        indices=($(seq 1 30))
    elif [[ "${selection}" =~ ^[0-9]+-[0-9]+$ ]]; then
        local start=${selection%-*}
        local end=${selection#*-}
        indices=($(seq "${start}" "${end}"))
    elif [[ "${selection}" =~ , ]]; then
        IFS=',' read -ra indices <<< "${selection}"
    else
        indices=("${selection}")
    fi

    echo "${indices[@]}"
}

run_single() {
    local particle=$1
    local dat_file=$2
    local max_events=${3:-}
    local dat_number=$(basename "${dat_file}" | sed 's/DAT//')

    echo "========================================================================"
    echo "Running: ${particle} DAT${dat_number}"
    echo "========================================================================"

    local output_dir="${OUTPUTS_BASE}/single_${particle}_DAT${dat_number}_$(date +%Y%m%d_%H%M%S)"
    mkdir -p "${output_dir}"

    local primaries_csv="${output_dir}/corsika_primaries.csv"
    local detector_hits="${output_dir}/detector_hits.txt"
    local run_log="${output_dir}/run.log"

    echo "  [1/2] Converting CORSIKA to Geant4 input..."
    if ! python3 "${SRC_DIR}/corsika_to_geant4.py" \
        "${dat_file}" \
        --output "${primaries_csv}" \
        >> "${run_log}" 2>&1; then
        echo "  ✗ FAILED: CORSIKA conversion (see ${run_log})"
        return 1
    fi

    local total_events=$(tail -n +2 "${primaries_csv}" | wc -l)
    if [[ ${total_events} -eq 0 ]]; then
        echo "  ✗ FAILED: No events in CSV"
        return 1
    fi

    local num_events=${total_events}
    if [[ -n "${max_events}" ]] && (( max_events < total_events )); then
        num_events=${max_events}
    fi

    if [[ -n "${max_events}" ]]; then
        echo "  [2/2] Running Geant4 (${num_events}/${total_events} events)..."
    else
        echo "  [2/2] Running Geant4 (${num_events} events)..."
    fi

    if ! (
        cd "${output_dir}"
        "${BUILD_DIR}/condorSim" --events "${num_events}" >> "${run_log}" 2>&1
    ); then
        echo "  ✗ FAILED: Geant4 error (see ${run_log})"
        tail -30 "${run_log}" | grep -A 20 "ERROR\|Exception" || true
        return 1
    fi

    if [[ ! -f "${detector_hits}" ]]; then
        echo "  ✗ FAILED: No detector_hits.txt produced"
        return 1
    fi

    local num_hits=$(grep -c "^HIT" "${detector_hits}" 2>/dev/null || echo "0")
    local num_events_with_hits=$(grep -c "^EVENT" "${detector_hits}" 2>/dev/null || echo "0")
    echo "  ✓ SUCCESS: ${num_events_with_hits} events, ${num_hits} hits"
}

# ============================================================================
# Main
# ============================================================================

PARTICLE_TYPE="${1:-}"
SELECTION="${2:-}"
NUM_EVENTS="${3:-}"

if [[ -z "${PARTICLE_TYPE}" ]]; then
    print_usage
    exit 1
fi

if [[ -n "${NUM_EVENTS}" ]] && ! [[ "${NUM_EVENTS}" =~ ^[0-9]+$ ]]; then
    echo "ERROR: num_events must be a positive integer" >&2
    exit 1
fi

# Handle all (both particle types)
if [[ "${PARTICLE_TYPE}" == "all" ]]; then
    echo "Running ALL simulations (60 files)"
    echo "========================================================================"

    local_total=0
    local_success=0
    local_failed=0
    start_time=$(date +%s)

    for particle in gammas protons; do
        for dat_file in "${CORSIKA_DIR}/${particle}"/DAT[0-9]*; do
            [[ -f "${dat_file}" ]] || continue
            [[ "${dat_file}" =~ \.(log|long)$ ]] && continue

            ((local_total+=1))
            if run_single "${particle}" "${dat_file}" "${NUM_EVENTS}"; then
                ((local_success+=1))
            else
                ((local_failed+=1))
            fi
        done
    done

    end_time=$(date +%s)
    elapsed=$((end_time - start_time))

    echo "========================================================================"
    echo "FINAL SUMMARY"
    echo "========================================================================"
    echo "Total:   ${local_total} simulations"
    echo "Success: ${local_success}"
    echo "Failed:  ${local_failed}"
    echo "Time:    ${elapsed} seconds"
    echo "========================================================================"

    exit $([[ ${local_failed} -eq 0 ]] && echo 0 || echo 1)
fi

if [[ "${PARTICLE_TYPE}" != "gammas" && "${PARTICLE_TYPE}" != "protons" ]]; then
    echo "ERROR: particle must be 'gammas', 'protons', or 'all'" >&2
    print_usage
    exit 1
fi

if [[ -z "${SELECTION}" ]]; then
    echo "ERROR: selection required for particle '${PARTICLE_TYPE}'" >&2
    print_usage
    exit 1
fi

indices=($(parse_selection "${SELECTION}"))

echo "Running ${#indices[@]} ${PARTICLE_TYPE} simulation(s)"
echo "Indices: ${indices[*]}"
echo "========================================================================"

total=0
success=0
failed=0
start_time=$(date +%s)

for index in "${indices[@]}"; do
    if (( index < 1 || index > 30 )); then
        echo "ERROR: Index ${index} out of range (1-30)" >&2
        ((failed+=1))
        continue
    fi

    dat_number=$(index_to_dat "${PARTICLE_TYPE}" "${index}")
    dat_file="${CORSIKA_DIR}/${PARTICLE_TYPE}/DAT${dat_number}"

    if [[ ! -f "${dat_file}" ]]; then
        echo "ERROR: File not found: ${dat_file}" >&2
        ((failed+=1))
        continue
    fi

    ((total+=1))
    if run_single "${PARTICLE_TYPE}" "${dat_file}" "${NUM_EVENTS}"; then
        ((success+=1))
    else
        ((failed+=1))
    fi
done

end_time=$(date +%s)
elapsed=$((end_time - start_time))

echo "========================================================================"
echo "SUMMARY"
echo "========================================================================"
echo "Completed: ${total} simulations"
echo "Success:   ${success}"
echo "Failed:    ${failed}"
echo "Time:      ${elapsed} seconds"
if (( total > 0 )); then
    echo "Avg:       $((elapsed / total)) sec/simulation"
fi
echo "========================================================================"

exit $([[ ${failed} -eq 0 ]] && echo 0 || echo 1)
