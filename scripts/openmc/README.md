# OpenMC AHWR-edu collapse

1. Install OpenMC (micromamba env used here: `.tools/mamba` env `openmc`).
2. Extract ENDF/B-VIII.0 neutron + thermal files under `.tools/nuclear_data/endf/`.
3. Build HDF5 subset: `python build_endfb80_subset.py`
4. Collapse: `python collapse_ahwr_edu_cell.py`
5. Apply into tree: `python apply_collapse_to_tree.py`

Geometry dimensions follow Kumar et al., IAEA-CN-164-3S03 Table I (educational
equal-area pin cell). Outputs land in `data/thorium/openmc_collapse/`.
