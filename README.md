# Isometric_Injective_Energies

Code release is in progress. Please stay tuned.

## Usage


### TLC_2D
Mapping triangle mesh by optimizing TLC energy
```
Usage: ./TLC_2D [OPTIONS] input_file [solver_options_file]

Positionals:
input_file TEXT REQUIRED    input data file
solver_options_file TEXT    solver options file

Options:
-h,--help                   Print this help message and exit
-I,--stop-at-injectivity    Stop optimization when injective map is found
```

### IsoTLC_2D
Mapping triangle mesh by optimizing IsoTLC energy
```
Usage: ./IsoTLC_2D [OPTIONS] input_file [solver_options_file]

Positionals:
input_file TEXT REQUIRED    input data file
solver_options_file TEXT    solver options file

Options:
-h,--help                   Print this help message and exit
-I,--stop-at-injectivity    Stop optimization when injective map is found
```