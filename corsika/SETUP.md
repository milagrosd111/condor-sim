# CORSIKA 7.8050 Local Setup (Not in Git)

CORSIKA is licensed separately and must be downloaded manually. The compiled tree lives at `corsika/corsika-78050/`, which is gitignored. These steps rebuild it locally without committing binaries.

## Prerequisites
- gfortran, gcc/g++
- make
- ~5 GB free disk space

## Steps
1) Request/download the CORSIKA 7.8050 tarball from the official site (KIT/IAP). Save it as `corsika-78050.tar.gz` in the project root.
2) Unpack into the expected path:
```bash
mkdir -p corsika
tar -xzf corsika-78050.tar.gz -C corsika
```
3) Configure with EPOS + UrQMD (matches project settings):
```bash
cd corsika/corsika-78050
./configure --with-epos --with-urqmd
```
4) Build the executable:
```bash
make -j4
```
5) Verify the binary exists:
```bash
ls run/corsika78050Linux_EPOS_urqmd
```
6) Keep the entire `corsika-78050` directory out of git (already ignored).

## Regenerating Outputs
CORSIKA DAT/LOG/LONG outputs are also generated and gitignored. Regenerate as needed using the run scripts in `corsika/simulation/`.
