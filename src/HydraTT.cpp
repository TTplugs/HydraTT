#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "lv2/atom/atom.h"
#include "lv2/atom/util.h"
#include "lv2/core/lv2.h"

static const char* kUri = "urn:asier:lv2:hydratt";

enum PortIndex : uint32_t {
  MIDI_IN = 0,
  OUT_L,
  OUT_R,
  OSC1_WAVE,
  OSC1_SHAPE,
  OSC1_PW,
  OSC1_OCT,
  OSC1_SEMI,
  OSC1_DETUNE,
  OSC2_WAVE,
  OSC2_LEVEL,
  OSC2_OCT,
  OSC2_SEMI,
  OSC2_DETUNE,
  OSC_SYNC,
  FM_AMOUNT,
  SUB_LEVEL,
  NOISE_LEVEL,
  MIX_BALANCE,
  FILTER_MODE,
  FILTER_CUTOFF,
  FILTER_RESONANCE,
  FILTER_ENV_AMT,
  ENV_ATTACK,
  ENV_DECAY,
  ENV_SUSTAIN,
  ENV_RELEASE,
  LFO_RATE,
  LFO_DEPTH,
  VELOCITY_AMT,
  DRIVE,
  GLIDE_MS
};

enum ParamIndex : uint32_t {
  P_OSC1_WAVE = 0,
  P_OSC1_SHAPE,
  P_OSC1_PW,
  P_OSC1_OCT,
  P_OSC1_SEMI,
  P_OSC1_DETUNE,
  P_OSC2_WAVE,
  P_OSC2_LEVEL,
  P_OSC2_OCT,
  P_OSC2_SEMI,
  P_OSC2_DETUNE,
  P_OSC_SYNC,
  P_FM_AMOUNT,
  P_SUB_LEVEL,
  P_NOISE_LEVEL,
  P_MIX_BALANCE,
  P_FILTER_MODE,
  P_FILTER_CUTOFF,
  P_FILTER_RESONANCE,
  P_FILTER_ENV_AMT,
  P_ENV_ATTACK,
  P_ENV_DECAY,
  P_ENV_SUSTAIN,
  P_ENV_RELEASE,
  P_LFO_RATE,
  P_LFO_DEPTH,
  P_VELOCITY_AMT,
  P_DRIVE,
  P_GLIDE_MS,
  kParamCount
};

struct ParamDef {
  float def;
  float min;
  float max;
};

static const ParamDef kParamDefs[kParamCount] = {
    {0.0f, 0.0f, 4.0f},        // OSC1_Wave
    {0.35f, 0.0f, 1.0f},       // OSC1_Shape
    {0.50f, 0.05f, 0.95f},     // OSC1_PW
    {0.0f, -2.0f, 2.0f},       // OSC1_Oct
    {0.0f, -12.0f, 12.0f},     // OSC1_Semi
    {0.0f, -50.0f, 50.0f},     // OSC1_Detune
    {4.0f, 0.0f, 4.0f},        // OSC2_Wave
    {0.80f, 0.0f, 1.0f},       // OSC2_Level
    {0.0f, -2.0f, 2.0f},       // OSC2_Oct
    {0.0f, -12.0f, 12.0f},     // OSC2_Semi
    {0.0f, -50.0f, 50.0f},     // OSC2_Detune
    {0.0f, 0.0f, 1.0f},        // OSC_Sync
    {0.20f, 0.0f, 1.0f},       // FM_Amount
    {0.20f, 0.0f, 1.0f},       // SUB_Level
    {0.03f, 0.0f, 1.0f},       // NOISE_Level
    {0.0f, -1.0f, 1.0f},       // MIX_Balance
    {0.0f, 0.0f, 2.0f},        // FILTER_Mode
    {9500.0f, 20.0f, 20000.0f},  // FILTER_Cutoff
    {0.22f, 0.0f, 1.0f},       // FILTER_Resonance
    {26.0f, -48.0f, 48.0f},    // FILTER_EnvAmt
    {3.0f, 0.5f, 5000.0f},     // ENV_Attack
    {520.0f, 1.0f, 8000.0f},   // ENV_Decay
    {0.72f, 0.0f, 1.0f},       // ENV_Sustain
    {950.0f, 1.0f, 12000.0f},  // ENV_Release
    {2.40f, 0.05f, 40.0f},     // LFO_Rate
    {0.28f, 0.0f, 1.0f},       // LFO_Depth
    {0.70f, 0.0f, 1.0f},       // Velocity_Amount
    {0.16f, 0.0f, 1.0f},       // Drive
    {0.0f, 0.0f, 500.0f}       // Glide_ms
};

static const uint8_t kDiscreteParam[kParamCount] = {
    1,  // OSC1_WAVE
    0,  // OSC1_SHAPE
    0,  // OSC1_PW
    1,  // OSC1_OCT
    1,  // OSC1_SEMI
    0,  // OSC1_DETUNE
    1,  // OSC2_WAVE
    0,  // OSC2_LEVEL
    1,  // OSC2_OCT
    1,  // OSC2_SEMI
    0,  // OSC2_DETUNE
    1,  // OSC_SYNC
    0,  // FM_AMOUNT
    0,  // SUB_LEVEL
    0,  // NOISE_LEVEL
    0,  // MIX_BALANCE
    1,  // FILTER_MODE
    0,  // FILTER_CUTOFF
    0,  // FILTER_RESONANCE
    0,  // FILTER_ENV_AMT
    0,  // ENV_ATTACK
    0,  // ENV_DECAY
    0,  // ENV_SUSTAIN
    0,  // ENV_RELEASE
    0,  // LFO_RATE
    0,  // LFO_DEPTH
    0,  // VELOCITY_AMT
    0,  // DRIVE
    0   // GLIDE_MS
};

enum EnvStage : int32_t {
  ENV_IDLE = 0,
  ENV_ATTACK_STAGE,
  ENV_DECAY_STAGE,
  ENV_SUSTAIN_STAGE,
  ENV_RELEASE_STAGE
};

struct Plugin {
  float sr;
  float* out_l;
  float* out_r;
  const LV2_Atom_Sequence* midi_in;
  const float* controls[kParamCount];
  float smooth[kParamCount];

  float osc1_phase;
  float osc2_phase;
  float sub_phase;
  float lfo_phase;
  float env;
  float velocity_gain;
  float pitch_bend;
  float note_hz_target;
  float note_hz_current;
  float svf_lp;
  float svf_bp;
  uint32_t rng;
  uint8_t active_note;
  int32_t stage;
};

static constexpr float kPi = 3.14159265358979323846f;
static constexpr float kTwoPi = 6.28318530717958647692f;
static constexpr float kRefSampleRate = 48000.0f;
static constexpr float kMinCutoff = 20.0f;
static constexpr float kNyquistSafety = 0.45f;

static bool finite_float(float x) {
  return __builtin_isfinite(x);
}

static bool finite_double(double x) {
  return __builtin_isfinite(x);
}

static float finite_or(float x, float fallback) {
  return finite_float(x) ? x : fallback;
}

static float clamp(float x, float lo, float hi) {
  x = finite_or(x, lo);
  return x < lo ? lo : (x > hi ? hi : x);
}

static float zap(float x) {
  x = finite_or(x, 0.0f);
  return fabsf(x) < 1.0e-20f ? 0.0f : x;
}

static int nearest_int(float x) {
  x = finite_or(x, 0.0f);
  return static_cast<int>(x + (x >= 0.0f ? 0.5f : -0.5f));
}

static float quantize_2(float x) {
  x = finite_or(x, 0.0f);
  const float scaled = x * 100.0f;
  const int32_t rounded = static_cast<int32_t>(scaled + (scaled >= 0.0f ? 0.5f : -0.5f));
  return static_cast<float>(rounded) * 0.01f;
}

static float sample_rate(const Plugin* p) {
  return (p && finite_float(p->sr) && p->sr >= 1000.0f && p->sr <= 384000.0f) ? p->sr : kRefSampleRate;
}

static float time_coef_ms(const Plugin* p, float ms) {
  ms = clamp(ms, 0.1f, 20000.0f);
  const float sr = sample_rate(p);
  return finite_or(expf(-1.0f / (0.001f * ms * sr)), 0.0f);
}

static float wrap_phase01(float x) {
  if (!finite_float(x)) return 0.0f;
  while (x >= 1.0f) x -= 1.0f;
  while (x < 0.0f) x += 1.0f;
  return x;
}

static float semitone_ratio(float semitones) {
  return finite_or(powf(2.0f, semitones / 12.0f), 1.0f);
}

static float midi_to_hz(int note) {
  note = note < 0 ? 0 : (note > 127 ? 127 : note);
  return finite_or(440.0f * powf(2.0f, (static_cast<float>(note) - 69.0f) / 12.0f), 440.0f);
}

static float soft_clip(float x) {
  x = finite_or(x, 0.0f);
  return finite_or(tanhf(x), 0.0f);
}

static int control_switch(float x) {
  return x >= 0.5f ? 1 : 0;
}

static uint32_t next_u32(Plugin* p) {
  p->rng = p->rng * 1664525u + 1013904223u;
  return p->rng;
}

static float frand(Plugin* p) {
  return ((next_u32(p) >> 8) & 0xFFFFu) / 32768.0f - 1.0f;
}

static float osc_wave(float phase, int wave, float shape, float pw) {
  phase = wrap_phase01(phase);
  wave = wave < 0 ? 0 : (wave > 4 ? 4 : wave);
  shape = clamp(shape, 0.0f, 1.0f);
  pw = clamp(pw, 0.05f, 0.95f);

  const float sine = sinf(kTwoPi * phase);
  const float saw = phase * 2.0f - 1.0f;
  const float tri = 1.0f - 4.0f * fabsf(phase - 0.5f);

  switch (wave) {
    case 0: {
      const float body = saw + sinf(kTwoPi * phase * 2.0f) * 0.35f * shape;
      return clamp(body, -1.0f, 1.0f);
    }
    case 1: {
      const float bend = soft_clip(tri * (1.0f + shape * 8.0f));
      return clamp(tri * (1.0f - shape) + bend * shape, -1.0f, 1.0f);
    }
    case 2: {
      const float width = clamp(pw + (shape - 0.5f) * 0.45f, 0.05f, 0.95f);
      return phase < width ? 1.0f : -1.0f;
    }
    case 3: {
      return clamp(sine + sinf(kTwoPi * phase * 2.0f) * shape * 0.25f, -1.0f, 1.0f);
    }
    default: {
      const float x = saw * (0.30f + 0.70f * shape) + sine;
      return clamp(soft_clip(x * (1.0f + shape * 10.0f)), -1.0f, 1.0f);
    }
  }
}

static float process_filter(Plugin* p, float x, float cutoff_hz, float resonance, int mode) {
  x = finite_or(x, 0.0f);
  const float sr = sample_rate(p);
  cutoff_hz = clamp(cutoff_hz, kMinCutoff, sr * kNyquistSafety);
  resonance = clamp(resonance, 0.0f, 1.0f);
  mode = mode < 0 ? 0 : (mode > 2 ? 2 : mode);

  const float f = clamp(2.0f * sinf(kPi * cutoff_hz / sr), 0.00001f, 0.95f);
  const float damping = clamp(1.55f - resonance * 1.45f, 0.10f, 1.55f);

  p->svf_lp += f * p->svf_bp;
  float hp = x - p->svf_lp - damping * p->svf_bp;
  p->svf_bp += f * hp;

  p->svf_lp = zap(clamp(p->svf_lp, -8.0f, 8.0f));
  p->svf_bp = zap(clamp(p->svf_bp, -8.0f, 8.0f));
  hp = zap(clamp(hp, -8.0f, 8.0f));

  switch (mode) {
    case 1:
      return finite_or(p->svf_bp, 0.0f);
    case 2:
      return finite_or(hp, 0.0f);
    default:
      return finite_or(p->svf_lp, 0.0f);
  }
}

static void init_plugin(Plugin* p, float sr) {
  memset(p, 0, sizeof(Plugin));
  p->sr = sr;
  p->rng = 0x48594452u;
  p->velocity_gain = 1.0f;
  p->note_hz_target = 110.0f;
  p->note_hz_current = 110.0f;
  p->active_note = 60u;
  p->stage = ENV_IDLE;
  for (uint32_t i = 0; i < kParamCount; ++i) {
    p->controls[i] = nullptr;
    p->smooth[i] = kParamDefs[i].def;
  }
}

static void reset_dsp_state(Plugin* p) {
  if (!p) return;
  p->osc1_phase = 0.0f;
  p->osc2_phase = 0.0f;
  p->sub_phase = 0.0f;
  p->lfo_phase = 0.0f;
  p->env = 0.0f;
  p->velocity_gain = 1.0f;
  p->pitch_bend = 0.0f;
  p->note_hz_target = 110.0f;
  p->note_hz_current = 110.0f;
  p->svf_lp = 0.0f;
  p->svf_bp = 0.0f;
  p->rng = 0x48594452u;
  p->active_note = 60u;
  p->stage = ENV_IDLE;
}

static void note_on(Plugin* p, uint8_t note, float velocity) {
  if (!p) return;
  p->active_note = note;
  p->note_hz_target = clamp(midi_to_hz(static_cast<int>(note)), 10.0f, sample_rate(p) * kNyquistSafety);
  if (p->stage == ENV_IDLE || p->smooth[P_GLIDE_MS] <= 0.001f) {
    p->note_hz_current = p->note_hz_target;
  }
  const float vel_amt = clamp(p->smooth[P_VELOCITY_AMT], 0.0f, 1.0f);
  velocity = clamp(velocity, 0.0f, 1.0f);
  p->velocity_gain = clamp((1.0f - vel_amt) + velocity * vel_amt, 0.0f, 1.0f);
  if (control_switch(p->smooth[P_OSC_SYNC])) {
    p->osc1_phase = 0.0f;
    p->osc2_phase = 0.0f;
    p->sub_phase = 0.0f;
  }
  p->stage = ENV_ATTACK_STAGE;
}

static void note_off(Plugin* p, uint8_t note) {
  if (!p) return;
  if (note == p->active_note || note > 127u) {
    if (p->stage != ENV_IDLE) {
      p->stage = ENV_RELEASE_STAGE;
    }
  }
}

static void handle_midi(Plugin* p) {
  if (!p || !p->midi_in) return;
  if (p->midi_in->atom.size < 8u) return;

  LV2_ATOM_SEQUENCE_FOREACH(p->midi_in, ev) {
    const uint8_t* msg = reinterpret_cast<const uint8_t*>(ev + 1);
    if (ev->body.size < 1u) continue;
    const uint8_t status = msg[0] & 0xF0u;

    if (status == 0x90u && ev->body.size >= 3u) {
      if (msg[2] > 0u) {
        note_on(p, msg[1], static_cast<float>(msg[2]) / 127.0f);
      } else {
        note_off(p, msg[1]);
      }
    } else if (status == 0x80u && ev->body.size >= 3u) {
      note_off(p, msg[1]);
    } else if (status == 0xE0u && ev->body.size >= 3u) {
      const int bend14 = (static_cast<int>(msg[2]) << 7) | static_cast<int>(msg[1]);
      p->pitch_bend = clamp(static_cast<float>(bend14 - 8192) / 8192.0f, -1.0f, 1.0f);
    } else if (status == 0xB0u && ev->body.size >= 3u) {
      if (msg[1] == 120u || msg[1] == 123u) {
        note_off(p, 255u);
      }
    }
  }
}

static void smooth_params(Plugin* p, float smooth_step) {
  for (uint32_t i = 0; i < kParamCount; ++i) {
    const ParamDef* def = &kParamDefs[i];
    float target = p->controls[i] ? *p->controls[i] : def->def;
    target = clamp(target, def->min, def->max);

    if (kDiscreteParam[i]) {
      target = clamp(static_cast<float>(nearest_int(target)), def->min, def->max);
      p->smooth[i] = target;
    } else {
      target = quantize_2(target);
      p->smooth[i] += (target - p->smooth[i]) * smooth_step;
      p->smooth[i] = zap(clamp(p->smooth[i], def->min, def->max));
    }
  }
}

static float update_env(Plugin* p, float atk_ms, float dec_ms, float sus, float rel_ms) {
  const float atk_step = 1.0f - time_coef_ms(p, atk_ms);
  const float dec_step = 1.0f - time_coef_ms(p, dec_ms);
  const float rel_step = 1.0f - time_coef_ms(p, rel_ms);
  sus = clamp(sus, 0.0f, 1.0f);

  switch (p->stage) {
    case ENV_ATTACK_STAGE:
      p->env += (1.0f - p->env) * atk_step;
      if (p->env >= 0.999f) {
        p->env = 1.0f;
        p->stage = ENV_DECAY_STAGE;
      }
      break;
    case ENV_DECAY_STAGE:
      p->env += (sus - p->env) * dec_step;
      if (fabsf(p->env - sus) < 0.0005f) {
        p->env = sus;
        p->stage = ENV_SUSTAIN_STAGE;
      }
      break;
    case ENV_SUSTAIN_STAGE:
      p->env = sus;
      break;
    case ENV_RELEASE_STAGE:
      p->env += (0.0f - p->env) * rel_step;
      if (p->env <= 0.00005f) {
        p->env = 0.0f;
        p->stage = ENV_IDLE;
      }
      break;
    default:
      p->env = 0.0f;
      break;
  }

  p->env = zap(clamp(p->env, 0.0f, 1.0f));
  return p->env;
}

static LV2_Handle instantiate(const LV2_Descriptor*, double rate, const char*, const LV2_Feature* const*) {
  Plugin* p = static_cast<Plugin*>(malloc(sizeof(Plugin)));
  if (!p) return nullptr;
  const float sr = (finite_double(rate) && rate >= 1000.0 && rate <= 384000.0) ? static_cast<float>(rate) : kRefSampleRate;
  init_plugin(p, sr);
  return p;
}

static void connect_port(LV2_Handle instance, uint32_t port, void* data) {
  Plugin* p = static_cast<Plugin*>(instance);
  if (!p) return;

  switch (port) {
    case MIDI_IN:
      p->midi_in = static_cast<const LV2_Atom_Sequence*>(data);
      break;
    case OUT_L:
      p->out_l = static_cast<float*>(data);
      break;
    case OUT_R:
      p->out_r = static_cast<float*>(data);
      break;
    case OSC1_WAVE:
      p->controls[P_OSC1_WAVE] = static_cast<const float*>(data);
      break;
    case OSC1_SHAPE:
      p->controls[P_OSC1_SHAPE] = static_cast<const float*>(data);
      break;
    case OSC1_PW:
      p->controls[P_OSC1_PW] = static_cast<const float*>(data);
      break;
    case OSC1_OCT:
      p->controls[P_OSC1_OCT] = static_cast<const float*>(data);
      break;
    case OSC1_SEMI:
      p->controls[P_OSC1_SEMI] = static_cast<const float*>(data);
      break;
    case OSC1_DETUNE:
      p->controls[P_OSC1_DETUNE] = static_cast<const float*>(data);
      break;
    case OSC2_WAVE:
      p->controls[P_OSC2_WAVE] = static_cast<const float*>(data);
      break;
    case OSC2_LEVEL:
      p->controls[P_OSC2_LEVEL] = static_cast<const float*>(data);
      break;
    case OSC2_OCT:
      p->controls[P_OSC2_OCT] = static_cast<const float*>(data);
      break;
    case OSC2_SEMI:
      p->controls[P_OSC2_SEMI] = static_cast<const float*>(data);
      break;
    case OSC2_DETUNE:
      p->controls[P_OSC2_DETUNE] = static_cast<const float*>(data);
      break;
    case OSC_SYNC:
      p->controls[P_OSC_SYNC] = static_cast<const float*>(data);
      break;
    case FM_AMOUNT:
      p->controls[P_FM_AMOUNT] = static_cast<const float*>(data);
      break;
    case SUB_LEVEL:
      p->controls[P_SUB_LEVEL] = static_cast<const float*>(data);
      break;
    case NOISE_LEVEL:
      p->controls[P_NOISE_LEVEL] = static_cast<const float*>(data);
      break;
    case MIX_BALANCE:
      p->controls[P_MIX_BALANCE] = static_cast<const float*>(data);
      break;
    case FILTER_MODE:
      p->controls[P_FILTER_MODE] = static_cast<const float*>(data);
      break;
    case FILTER_CUTOFF:
      p->controls[P_FILTER_CUTOFF] = static_cast<const float*>(data);
      break;
    case FILTER_RESONANCE:
      p->controls[P_FILTER_RESONANCE] = static_cast<const float*>(data);
      break;
    case FILTER_ENV_AMT:
      p->controls[P_FILTER_ENV_AMT] = static_cast<const float*>(data);
      break;
    case ENV_ATTACK:
      p->controls[P_ENV_ATTACK] = static_cast<const float*>(data);
      break;
    case ENV_DECAY:
      p->controls[P_ENV_DECAY] = static_cast<const float*>(data);
      break;
    case ENV_SUSTAIN:
      p->controls[P_ENV_SUSTAIN] = static_cast<const float*>(data);
      break;
    case ENV_RELEASE:
      p->controls[P_ENV_RELEASE] = static_cast<const float*>(data);
      break;
    case LFO_RATE:
      p->controls[P_LFO_RATE] = static_cast<const float*>(data);
      break;
    case LFO_DEPTH:
      p->controls[P_LFO_DEPTH] = static_cast<const float*>(data);
      break;
    case VELOCITY_AMT:
      p->controls[P_VELOCITY_AMT] = static_cast<const float*>(data);
      break;
    case DRIVE:
      p->controls[P_DRIVE] = static_cast<const float*>(data);
      break;
    case GLIDE_MS:
      p->controls[P_GLIDE_MS] = static_cast<const float*>(data);
      break;
  }
}

static void activate(LV2_Handle instance) {
  reset_dsp_state(static_cast<Plugin*>(instance));
}

static void run(LV2_Handle instance, uint32_t nframes) {
  Plugin* p = static_cast<Plugin*>(instance);
  if (!p || !p->out_l || !p->out_r) return;

  handle_midi(p);

  const float sr = sample_rate(p);
  const float smooth_step = 1.0f - time_coef_ms(p, 8.0f);

  for (uint32_t i = 0; i < nframes; ++i) {
    smooth_params(p, smooth_step);

    const float atk_ms = p->smooth[P_ENV_ATTACK];
    const float dec_ms = p->smooth[P_ENV_DECAY];
    const float sus = p->smooth[P_ENV_SUSTAIN];
    const float rel_ms = p->smooth[P_ENV_RELEASE];
    const float env = update_env(p, atk_ms, dec_ms, sus, rel_ms);

    const float glide_ms = p->smooth[P_GLIDE_MS];
    if (glide_ms > 0.001f) {
      const float glide_step = 1.0f - time_coef_ms(p, glide_ms);
      p->note_hz_current += (p->note_hz_target - p->note_hz_current) * glide_step;
    } else {
      p->note_hz_current = p->note_hz_target;
    }

    const float base_hz = clamp(p->note_hz_current * semitone_ratio(p->pitch_bend * 2.0f), 10.0f, sr * kNyquistSafety);
    const float osc1_pitch = static_cast<float>(nearest_int(p->smooth[P_OSC1_OCT])) * 12.0f + static_cast<float>(nearest_int(p->smooth[P_OSC1_SEMI])) + p->smooth[P_OSC1_DETUNE] * 0.01f;
    const float osc2_pitch = static_cast<float>(nearest_int(p->smooth[P_OSC2_OCT])) * 12.0f + static_cast<float>(nearest_int(p->smooth[P_OSC2_SEMI])) + p->smooth[P_OSC2_DETUNE] * 0.01f;

    float osc1_hz = clamp(base_hz * semitone_ratio(osc1_pitch), 10.0f, sr * kNyquistSafety);
    float osc2_hz = clamp(base_hz * semitone_ratio(osc2_pitch), 10.0f, sr * kNyquistSafety);
    float sub_hz = clamp(base_hz * 0.5f, 10.0f, sr * kNyquistSafety);

    p->lfo_phase = wrap_phase01(p->lfo_phase + p->smooth[P_LFO_RATE] / sr);
    const float lfo = sinf(kTwoPi * p->lfo_phase);
    const float lfo_depth = clamp(p->smooth[P_LFO_DEPTH], 0.0f, 1.0f);
    const float pw_mod = clamp(p->smooth[P_OSC1_PW] + lfo * lfo_depth * 0.20f, 0.05f, 0.95f);

    const float osc1_prev = p->osc1_phase;
    p->osc1_phase = wrap_phase01(p->osc1_phase + osc1_hz / sr);
    const bool osc1_wrapped = p->osc1_phase < osc1_prev;
    if (osc1_wrapped && control_switch(p->smooth[P_OSC_SYNC])) {
      p->osc2_phase = 0.0f;
    } else {
      p->osc2_phase = wrap_phase01(p->osc2_phase + osc2_hz / sr);
    }
    p->sub_phase = wrap_phase01(p->sub_phase + sub_hz / sr);

    const int wave2 = nearest_int(p->smooth[P_OSC2_WAVE]);
    const float osc2 = osc_wave(p->osc2_phase, wave2, p->smooth[P_OSC1_SHAPE], pw_mod);
    const float fm = osc2 * (p->smooth[P_FM_AMOUNT] * p->smooth[P_FM_AMOUNT] * 0.18f);
    const int wave1 = nearest_int(p->smooth[P_OSC1_WAVE]);
    const float osc1 = osc_wave(wrap_phase01(p->osc1_phase + fm), wave1, p->smooth[P_OSC1_SHAPE], pw_mod);
    const float sub = sinf(kTwoPi * p->sub_phase);
    const float noise = frand(p) * 0.45f;

    const float bal = clamp(p->smooth[P_MIX_BALANCE], -1.0f, 1.0f);
    const float w1 = 0.5f * (1.0f - bal);
    const float w2 = 0.5f * (1.0f + bal);
    float signal = osc1 * w1 + osc2 * w2 * p->smooth[P_OSC2_LEVEL] + sub * p->smooth[P_SUB_LEVEL] + noise * p->smooth[P_NOISE_LEVEL];

    signal *= env * p->velocity_gain;

    const float env_cut = p->smooth[P_FILTER_ENV_AMT] * env;
    const float lfo_cut = lfo * lfo_depth * 12.0f;
    const float cutoff = clamp(p->smooth[P_FILTER_CUTOFF] * semitone_ratio(env_cut + lfo_cut), kMinCutoff, sr * kNyquistSafety);
    const int fmode = nearest_int(p->smooth[P_FILTER_MODE]);
    signal = process_filter(p, signal, cutoff, p->smooth[P_FILTER_RESONANCE], fmode);

    const float drive = clamp(p->smooth[P_DRIVE], 0.0f, 1.0f);
    if (drive > 0.0001f) {
      signal = soft_clip(signal * (1.0f + drive * 10.0f)) / (1.0f + drive * 0.8f);
    }
    signal = clamp(finite_or(signal, 0.0f), -1.0f, 1.0f);

    p->out_l[i] = signal;
    p->out_r[i] = signal;
  }
}

static void cleanup(LV2_Handle instance) {
  free(instance);
}

static const LV2_Descriptor descriptor = {kUri, instantiate, connect_port, activate, run, nullptr, cleanup, nullptr};

extern "C" LV2_SYMBOL_EXPORT const LV2_Descriptor* lv2_descriptor(uint32_t index) {
  return index == 0 ? &descriptor : nullptr;
}
