# Poster Plot Export Guide

This guide details which plots to export from each analysis notebook for the academic poster.

## Required Plots for Poster (6 total)

### 1. Detector Geometry
**Source**: `notebooks/detection/detector_array_simulation.ipynb`
- **Cell**: Cell #9 (Geometry Visualization section)
- **Filename**: `figures/detector_geometry.pdf`
- **Description**: 3D visualization of CONDOR detector array showing 120 units (100 central blue + 20 peripheral cyan)
- **Poster Location**: Motivation box (left column, top)
- **Export Command**:
  ```python
  plt.savefig('figures/detector_geometry.pdf', dpi=300, bbox_inches='tight')
  ```

### 2. Particle Composition Comparison
**Source**: `notebooks/cosmic_rays/corsika_simulation.ipynb`
- **Cell**: Cell #12 (Particle Composition section)
- **Filename**: `figures/particle_composition.pdf`
- **Description**: Bar charts comparing particle types in gamma vs. proton showers (muons, electrons, photons, hadrons). Shows <5% muons in gamma vs >20% in proton showers
- **Poster Location**: Simulation Chain box (left column, middle)
- **Key Insight**: Demonstrates primary discrimination mechanism between signal and background
- **Export Command**:
  ```python
  plt.savefig('figures/particle_composition.pdf', dpi=300, bbox_inches='tight')
  ```

### 3. Energy Linearity Validation
**Source**: `notebooks/detection/optical_physics.ipynb`
- **Cell**: Cell #13 (Section 4.5 - Energy Linearity)
- **Filename**: `figures/energy_linearity.pdf`
- **Description**: Two-panel plot showing:
  - Left: Energy vs. PE scatter plot with linear fit (R² > 0.96)
  - Right: Energy resolution as function of energy (16-18% across range)
- **Poster Location**: Light Yield Validation box (left column, bottom)
- **Key Metrics**: R²(gamma) = 0.973, R²(proton) = 0.968, validates 12.8 PE/MeV target
- **Export Command**:
  ```python
  plt.savefig('figures/energy_linearity.pdf', dpi=300, bbox_inches='tight')
  ```

### 4. Particle Comparison (Gamma vs. Proton)
**Source**: `notebooks/detection/geant4_simulation.ipynb`
- **Cell**: Cell #16 (Particle Comparison section)
- **Filename**: `figures/particle_comparison.pdf`
- **Description**: 2×2 grid showing:
  - Top-left: Energy deposition distributions
  - Top-right: Photoelectron yield distributions
  - Bottom-left: Hit multiplicity (bars activated)
  - Bottom-right: Light yield per bar
- **Poster Location**: Detector Performance box (right column, top section)
- **Key Insight**: Hit multiplicity provides 1.7× discrimination factor (15.3 vs. 26.1 bars)
- **Export Command**:
  ```python
  plt.savefig('figures/particle_comparison.pdf', dpi=300, bbox_inches='tight')
  ```

### 5. Timing Distribution
**Source**: `notebooks/detection/optical_physics.ipynb`
- **Cell**: Cell #15 (Section 5 - Timing Analysis)
- **Filename**: `figures/timing_distribution.pdf`
- **Description**: Two-panel histogram showing photon arrival times:
  - Left: Full distribution (0-50 ns)
  - Right: Zoom on prompt peak (0-10 ns, μ=2.8 ns, σ=1.2 ns)
- **Poster Location**: Detector Performance box (right column, middle section)
- **Key Insight**: Fast timing enables coincidence analysis for shower reconstruction
- **Export Command**:
  ```python
  plt.savefig('figures/timing_distribution.pdf', dpi=300, bbox_inches='tight')
  ```

### 6. ADC Spectrum
**Source**: `notebooks/detection/geant4_simulation.ipynb`
- **Cell**: Cell #9 (ADC Spectrum section)
- **Filename**: `figures/adc_spectrum.pdf`
- **Description**: Log-scale histogram of ADC values showing:
  - MIP peak for calibration
  - Extended tail from high-energy shower particles
  - Dynamic range 0.1-100 MeV
- **Poster Location**: Detector Performance box (right column, bottom section)
- **Key Insight**: Validates energy deposition model and detector dynamic range
- **Export Command**:
  ```python
  plt.savefig('figures/adc_spectrum.pdf', dpi=300, bbox_inches='tight')
  ```

---

## Export Workflow

### Step 1: Create figures directory
```bash
mkdir -p /home/milagros-delgado/Documents/SAPHIR/condor-sim/figures
```

### Step 2: Add export commands to each notebook

In each notebook cell that produces the plots listed above, add the save command immediately after `plt.tight_layout()` and before `plt.show()`:

```python
plt.tight_layout()
plt.savefig('figures/plot_name.pdf', dpi=300, bbox_inches='tight')
plt.show()
```

### Step 3: Execute relevant cells

Run each of the 6 cells listed above to generate the PDF plots.

### Step 4: Verify exports

Check that all 6 PDFs are created:
```bash
ls -lh /home/milagros-delgado/Documents/SAPHIR/condor-sim/figures/
```

Expected files:
- `detector_geometry.pdf`
- `particle_composition.pdf`
- `energy_linearity.pdf`
- `particle_comparison.pdf`
- `timing_distribution.pdf`
- `adc_spectrum.pdf`

---

## Alternative Plots (Optional)

If space permits or for backup options:

### 7. Light Yield Comparison (2-panel)
**Source**: `optical_physics.ipynb`, Cell #7
**Description**: Side-by-side histograms of light yield for gamma vs. proton
**Use Case**: Alternative to energy linearity if simpler visualization needed

### 8. Hit Multiplicity
**Source**: `geant4_simulation.ipynb`, Cell #7
**Description**: Histogram comparing number of bars hit per event (gamma vs. proton)
**Use Case**: More detailed view of discrimination capability shown in particle comparison

### 9. Detector Coverage
**Source**: `optical_physics.ipynb`, Cell #11
**Description**: Log-scale histogram showing hits per detector bar
**Use Case**: Demonstrates uniform detector response and spatial coverage

---

## Plot Styling Notes

All plots already use the academic poster styling with:
- **Colors**: Paul Tol colorblind-friendly palette (Gamma=#EE7733, Proton=#0077BB)
- **Fonts**: Title 16pt, Labels 14pt, Text 13pt
- **Quality**: 300 DPI for publication
- **Style**: No gridlines, bold labels, clean backgrounds

No additional styling adjustments needed before export.

---

## LaTeX Compilation

After exporting plots:

1. Update the `\includegraphics` paths in `condor_poster.tex` if needed
2. Ensure logo files are available in `logos/` directory
3. Compile with:
   ```bash
   pdflatex condor_poster.tex
   ```

4. If references/bibliography needed:
   ```bash
   pdflatex condor_poster.tex
   bibtex condor_poster
   pdflatex condor_poster.tex
   pdflatex condor_poster.tex
   ```
