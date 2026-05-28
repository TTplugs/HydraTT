> **WIP:** HydraTT is an early S2400-focused LV2 instrument prototype.

# HydraTT for S2400

HydraTT is an original polyphonic-leaning digital synth voice inspired by modern wavetable/modulation workflows.  
It does not copy proprietary Hydrasynth firmware, presets, branding, panel artwork, or architecture.

## S2400 Scope

- LV2 instrument (`lv2:InstrumentPlugin`)
- aarch64 / ARM64 Linux target
- MIDI input + stereo audio output
- MIDI Atom port at index `0`, audio outputs at indices `1` and `2`
- No audio input
- No GUI extension
- 29 control ports (`index 3..31`), below S2400 32-control limit
- Continuous controls are internally quantized to `0.01`

## Build on Ubuntu ARM64

```bash
cd ~/HydraTT
make clean
make
make check
```

Bundle output:

```text
HydraTT.lv2
```

Expected verification commands:

```bash
file HydraTT.lv2/HydraTT.so
readelf -d HydraTT.lv2/HydraTT.so | grep NEEDED
strings -a HydraTT.lv2/HydraTT.so | grep -E 'GLIBC_|GLIBCXX_|GCC_' | sort -V | uniq
nm -D HydraTT.lv2/HydraTT.so | grep lv2_descriptor
```

Optional if validators exist:

```bash
lv2_validate HydraTT.lv2/manifest.ttl HydraTT.lv2/HydraTT.ttl
LV2_PATH=. lv2lint -Mpack urn:asier:lv2:hydratt
```

## Copy to Unraid / S2400 Scan Path

```bash
ssh root@[IP] 'rm -rf "/mnt/user/Musica/Desarrollo LV2/HydraTT.lv2"'
scp -r HydraTT.lv2 root@[IP]:'/mnt/user/Musica/Desarrollo LV2/'
```

## Port Order

- `0`: `midi_in` (atom:AtomPort, atom:Sequence, midi:MidiEvent, lv2:designation lv2:control)
- `1`: `out_l` (audio out)
- `2`: `out_r` (audio out)
- `3`: `osc1_wave`
- `4`: `osc1_shape`
- `5`: `osc1_pw`
- `6`: `osc1_oct`
- `7`: `osc1_semi`
- `8`: `osc1_detune`
- `9`: `osc2_wave`
- `10`: `osc2_level`
- `11`: `osc2_oct`
- `12`: `osc2_semi`
- `13`: `osc2_detune`
- `14`: `osc_sync`
- `15`: `fm_amount`
- `16`: `sub_level`
- `17`: `noise_level`
- `18`: `mix_balance`
- `19`: `filter_mode`
- `20`: `filter_cutoff`
- `21`: `filter_resonance`
- `22`: `filter_env_amt`
- `23`: `env_attack`
- `24`: `env_decay`
- `25`: `env_sustain`
- `26`: `env_release`
- `27`: `lfo_rate`
- `28`: `lfo_depth`
- `29`: `velocity_amt`
- `30`: `drive`
- `31`: `glide_ms`

## Parameter Notes

- `osc1_wave`, `osc2_wave`: integer/enumeration (`Saw`, `Triangle`, `Pulse`, `Sine`, `Mutant`)
- `osc1_oct`, `osc2_oct`: integer/enumeration (`-2..2`)
- `osc1_semi`, `osc2_semi`: integer (`-12..12`)
- `osc_sync`: integer/toggled
- `filter_mode`: integer/enumeration (`LowPass`, `BandPass`, `HighPass`)
- `filter_cutoff`, ADSR times, and `lfo_rate`: logarithmic controls in TTL

## Starting Recipes

All values below are listed in control-port order (`3..31`).

### Glass Sweep

- `osc1_wave`: `4`
- `osc1_shape`: `0.62`
- `osc1_pw`: `0.50`
- `osc1_oct`: `0`
- `osc1_semi`: `0`
- `osc1_detune`: `0.00`
- `osc2_wave`: `3`
- `osc2_level`: `0.72`
- `osc2_oct`: `1`
- `osc2_semi`: `7`
- `osc2_detune`: `6.00`
- `osc_sync`: `0`
- `fm_amount`: `0.38`
- `sub_level`: `0.00`
- `noise_level`: `0.04`
- `mix_balance`: `0.10`
- `filter_mode`: `0`
- `filter_cutoff`: `7200.00`
- `filter_resonance`: `0.28`
- `filter_env_amt`: `34.00`
- `env_attack`: `18.00`
- `env_decay`: `1200.00`
- `env_sustain`: `0.62`
- `env_release`: `1800.00`
- `lfo_rate`: `1.20`
- `lfo_depth`: `0.42`
- `velocity_amt`: `0.78`
- `drive`: `0.10`
- `glide_ms`: `16.00`

### Neon Bass

- `osc1_wave`: `0`
- `osc1_shape`: `0.35`
- `osc1_pw`: `0.48`
- `osc1_oct`: `-1`
- `osc1_semi`: `0`
- `osc1_detune`: `-3.00`
- `osc2_wave`: `2`
- `osc2_level`: `0.64`
- `osc2_oct`: `-1`
- `osc2_semi`: `7`
- `osc2_detune`: `2.00`
- `osc_sync`: `1`
- `fm_amount`: `0.20`
- `sub_level`: `0.44`
- `noise_level`: `0.01`
- `mix_balance`: `-0.12`
- `filter_mode`: `0`
- `filter_cutoff`: `1800.00`
- `filter_resonance`: `0.34`
- `filter_env_amt`: `18.00`
- `env_attack`: `2.00`
- `env_decay`: `420.00`
- `env_sustain`: `0.74`
- `env_release`: `260.00`
- `lfo_rate`: `0.40`
- `lfo_depth`: `0.09`
- `velocity_amt`: `0.62`
- `drive`: `0.28`
- `glide_ms`: `24.00`

### Hyper Pad

- `osc1_wave`: `1`
- `osc1_shape`: `0.54`
- `osc1_pw`: `0.53`
- `osc1_oct`: `0`
- `osc1_semi`: `0`
- `osc1_detune`: `-5.00`
- `osc2_wave`: `4`
- `osc2_level`: `0.88`
- `osc2_oct`: `0`
- `osc2_semi`: `12`
- `osc2_detune`: `7.00`
- `osc_sync`: `0`
- `fm_amount`: `0.31`
- `sub_level`: `0.15`
- `noise_level`: `0.06`
- `mix_balance`: `0.22`
- `filter_mode`: `1`
- `filter_cutoff`: `6400.00`
- `filter_resonance`: `0.46`
- `filter_env_amt`: `29.00`
- `env_attack`: `40.00`
- `env_decay`: `1600.00`
- `env_sustain`: `0.68`
- `env_release`: `2500.00`
- `lfo_rate`: `3.10`
- `lfo_depth`: `0.37`
- `velocity_amt`: `0.80`
- `drive`: `0.12`
- `glide_ms`: `8.00`
