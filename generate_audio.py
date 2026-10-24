import wave
import struct
import math
import random
import os

OUTPUT_DIR = os.path.join(os.path.dirname(__file__), "audio")
os.makedirs(OUTPUT_DIR, exist_ok=True)
SAMPLE_RATE = 44100

def clamp(val, low=-1.0, high=1.0):
    return max(low, min(high, val))

def save_wav_stereo(filename, left_samples, right_samples, sample_rate=SAMPLE_RATE):
    filepath = os.path.join(OUTPUT_DIR, filename)
    # Peak normalize if exceeds 0.88
    max_peak = max(max(abs(s) for s in left_samples), max(abs(s) for s in right_samples), 0.001)
    scale = 0.85 / max_peak if max_peak > 0.85 else 1.0

    with wave.open(filepath, 'w') as wav:
        wav.setnchannels(2)
        wav.setsampwidth(2)
        wav.setframerate(sample_rate)
        frames = bytearray()
        for l, r in zip(left_samples, right_samples):
            il = int(clamp(l * scale) * 32767)
            ir = int(clamp(r * scale) * 32767)
            frames.extend(struct.pack('<hh', il, ir))
        wav.writeframes(frames)
    print(f"Generated {filename}: {len(left_samples)} samples ({len(left_samples)/sample_rate:.2f}s)")

def save_wav_mono(filename, samples, sample_rate=SAMPLE_RATE):
    save_wav_stereo(filename, samples, samples, sample_rate)

# --- Sound Synthesis Helpers ---
def note_freq(semitones_from_a4):
    return 440.0 * (2.0 ** (semitones_from_a4 / 12.0))

# Note offsets from A4 (A4 = 0, C4 = -9, C5 = 3, etc.)
NOTES = {
    'C3': -21, 'D3': -19, 'E3': -17, 'F3': -16, 'G3': -14, 'A3': -12, 'B3': -10,
    'C4': -9,  'D4': -7,  'E4': -5,  'F4': -4,  'G4': -2,  'A4': 0,   'B4': 2,
    'C5': 3,   'D5': 5,   'E5': 7,   'F5': 8,   'G5': 10,  'A5': 12,  'B5': 14,
    'C6': 15,  'D6': 17,  'E6': 19,  'F6': 20,  'G6': 22,  'A6': 24,  'B6': 26,
    'C7': 27,  'E7': 31,  'G7': 34,
}

# 1. SFX: Regular Egg Catch (Crisp pleasant coin/ding)
def gen_sfx_catch_regular():
    dur = 0.18
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 22.0)
        # Rising frequency from 587Hz to 920Hz
        freq = 587.0 + 333.0 * (1.0 - math.exp(-t * 30.0))
        # Sine + soft harmonic
        val = 0.7 * math.sin(2.0 * math.pi * freq * t) + 0.3 * math.sin(4.0 * math.pi * freq * t)
        samples.append(val * env)
    save_wav_mono("sfx_catch_regular.wav", samples)

# 2. SFX: Golden Egg Catch (Sparkling magic chime)
def gen_sfx_catch_golden():
    dur = 0.40
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    arpeggio = [note_freq(NOTES['E6']), note_freq(NOTES['G6']), note_freq(NOTES['B6']), note_freq(NOTES['E7'])]
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        val = 0.0
        for idx, base_freq in enumerate(arpeggio):
            start_t = idx * 0.04
            if t >= start_t:
                local_t = t - start_t
                env = math.exp(-local_t * 9.0)
                vibrato = 1.0 + 0.006 * math.sin(2.0 * math.pi * 7.0 * local_t)
                f = base_freq * vibrato
                s = math.sin(2.0 * math.pi * f * local_t) + 0.35 * math.sin(4.0 * math.pi * f * local_t)
                val += s * env * 0.3
        samples.append(val)
    save_wav_mono("sfx_catch_golden.wav", samples)

# 3. SFX: Basket Grow Egg Catch (Ascending powerup chime)
def gen_sfx_catch_grow():
    dur = 0.45
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    notes = [note_freq(NOTES['C5']), note_freq(NOTES['E5']), note_freq(NOTES['G5']), note_freq(NOTES['C6'])]
    step = 0.065
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        val = 0.0
        for idx, base_freq in enumerate(notes):
            st = idx * step
            if t >= st:
                lt = t - st
                env = math.exp(-lt * 10.0)
                s = math.sin(2.0 * math.pi * base_freq * lt) + 0.25 * math.sin(6.0 * math.pi * base_freq * lt)
                val += s * env * 0.35
        samples.append(val)
    save_wav_mono("sfx_catch_grow.wav", samples)

# 4. SFX: Legendary Phoenix Egg Catch (Triumphant celestial chime)
def gen_sfx_catch_legendary():
    dur = 1.10
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    # Major 9th celestial chord arpeggio
    freqs = [554.37, 659.25, 830.61, 987.77, 1318.51, 1661.22] # C#5, E5, G#5, B5, E6, G#6
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        val = 0.0
        for idx, bf in enumerate(freqs):
            st = idx * 0.07
            if t >= st:
                lt = t - st
                env = math.exp(-lt * 4.0)
                shimmer = math.sin(2.0 * math.pi * 5.0 * lt) * 0.008
                f = bf * (1.0 + shimmer)
                s = 0.6 * math.sin(2.0 * math.pi * f * lt) + 0.25 * math.sin(4.0 * math.pi * f * lt) + 0.15 * math.sin(6.0 * math.pi * f * lt)
                val += s * env * 0.25
        samples.append(val)
    save_wav_mono("sfx_catch_legendary.wav", samples)

# 5. SFX: Egg Miss / Splat
def gen_sfx_splat():
    dur = 0.24
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    random.seed(42)
    noise_filter = 0.0
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 18.0)
        # Low frequency impact thud (160 -> 40Hz)
        thud_f = 160.0 * math.exp(-t * 20.0) + 40.0
        thud = 0.6 * math.sin(2.0 * math.pi * thud_f * t)
        # Wet squelch noise burst
        white = (random.random() * 2.0 - 1.0)
        noise_filter = 0.75 * noise_filter + 0.25 * white
        wet_noise = noise_filter * math.exp(-t * 25.0) * 0.5
        samples.append((thud + wet_noise) * env)
    save_wav_mono("sfx_splat.wav", samples)

# 6. SFX: Bird Poop Hit Basket (Oof/squish thud + warning hit)
def gen_sfx_poop_hit():
    dur = 0.38
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    random.seed(77)
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 12.0)
        # Low comical squelch
        f_squelch = 320.0 * math.exp(-t * 16.0) + 70.0
        squelch = 0.5 * math.sin(2.0 * math.pi * f_squelch * t)
        # Dissonant buzz hit
        buzz = 0.3 * (1.0 if math.sin(2.0 * math.pi * 140.0 * t) > 0 else -1.0) * math.exp(-t * 15.0)
        samples.append((squelch + buzz) * env)
    save_wav_mono("sfx_poop_hit.wav", samples)

# 7. SFX: Bird Poop Hit Ground (Sticky slime puddle)
def gen_sfx_poop_ground():
    dur = 0.22
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 16.0)
        # Bubble resonance
        f = 240.0 * math.exp(-t * 12.0) + 80.0
        bubble = math.sin(2.0 * math.pi * f * t + 0.6 * math.sin(2.0 * math.pi * 24.0 * t))
        samples.append(bubble * env * 0.7)
    save_wav_mono("sfx_poop_ground.wav", samples)

# 8. SFX: Thunder & Lightning Crash
def gen_sfx_thunder():
    dur = 1.35
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    random.seed(99)
    rumble_filter = 0.0
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        # Crack at onset
        crack_env = math.exp(-t * 40.0) if t < 0.15 else 0.0
        crack = (random.random() * 2.0 - 1.0) * crack_env * 0.9
        # Deep low rumble
        rumble_env = (t / 0.1) * math.exp(-t * 3.5) if t < 0.1 else math.exp(-(t - 0.1) * 3.0)
        white = (random.random() * 2.0 - 1.0)
        rumble_filter = 0.94 * rumble_filter + 0.06 * white
        rumble_sine = math.sin(2.0 * math.pi * (50.0 + 15.0 * math.sin(t * 10.0)) * t)
        rumble = (rumble_filter * 0.6 + rumble_sine * 0.4) * rumble_env * 0.8
        samples.append(crack + rumble)
    save_wav_mono("sfx_thunder.wav", samples)

# 9. SFX: Electric Shock / Revival Zap
def gen_sfx_shock():
    dur = 0.55
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    random.seed(123)
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 6.0)
        jitter = random.uniform(-10.0, 10.0)
        f = 85.0 + jitter
        zap1 = 0.5 * (1.0 if math.sin(2.0 * math.pi * f * t) > 0 else -1.0)
        zap2 = 0.3 * (1.0 if math.sin(2.0 * math.pi * (f * 2.2) * t) > 0 else -1.0)
        spark = (random.random() * 2.0 - 1.0) * 0.25 if random.random() < 0.15 else 0.0
        samples.append((zap1 + zap2 + spark) * env * 0.7)
    save_wav_mono("sfx_shock.wav", samples)

# 10. SFX: Level Up Fanfare
def gen_sfx_level_up():
    dur = 0.70
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    melody = [
        (note_freq(NOTES['C5']), 0.00, 0.10),
        (note_freq(NOTES['E5']), 0.08, 0.10),
        (note_freq(NOTES['G5']), 0.16, 0.10),
        (note_freq(NOTES['B5']), 0.24, 0.10),
        (note_freq(NOTES['C6']), 0.32, 0.38),
        (note_freq(NOTES['E6']), 0.32, 0.38),
    ]
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        val = 0.0
        for f, st, note_dur in melody:
            if st <= t < st + note_dur + 0.1:
                lt = t - st
                env = math.exp(-lt * 8.0) if lt > 0.01 else lt / 0.01
                s = 0.7 * math.sin(2.0 * math.pi * f * lt) + 0.3 * math.sin(4.0 * math.pi * f * lt)
                val += s * env * 0.28
        samples.append(val)
    save_wav_mono("sfx_level_up.wav", samples)

# 11. SFX: Game Over (Melancholic soft decay)
def gen_sfx_game_over():
    dur = 0.95
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    melody = [
        (note_freq(NOTES['G4']), 0.00, 0.18),
        (note_freq(NOTES['E4']), 0.18, 0.18),
        (note_freq(NOTES['C4']), 0.36, 0.22),
        (note_freq(NOTES['A3']), 0.58, 0.37),
    ]
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        val = 0.0
        for f, st, note_dur in melody:
            if st <= t < st + note_dur + 0.15:
                lt = t - st
                env = math.exp(-lt * 6.0)
                # Triangle wave for soft melancholic feel
                phase = (f * lt) % 1.0
                tri = 4.0 * abs(phase - 0.5) - 1.0
                val += tri * env * 0.35
        samples.append(val)
    save_wav_mono("sfx_game_over.wav", samples)

# 12. SFX: Basket Growth Key [H] Use (Resonant swoosh expansion)
def gen_sfx_grow_use():
    dur = 0.38
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 9.0)
        # Rising resonant sweep 240Hz -> 820Hz
        freq = 240.0 + 580.0 * (t / dur) ** 1.5
        val = 0.6 * math.sin(2.0 * math.pi * freq * t) + 0.3 * math.sin(4.0 * math.pi * freq * t)
        samples.append(val * env * 0.75)
    save_wav_mono("sfx_grow_use.wav", samples)

# 13. SFX: Companion Basket [C] Summon (Futuristic retro companion chime)
def gen_sfx_companion_spawn():
    dur = 0.45
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    beeps = [
        (note_freq(NOTES['E5']), 0.00, 0.10),
        (note_freq(NOTES['A5']), 0.10, 0.10),
        (note_freq(NOTES['C6']), 0.20, 0.12),
        (note_freq(NOTES['E6']), 0.30, 0.15),
    ]
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        val = 0.0
        for f, st, nd in beeps:
            if st <= t < st + nd + 0.05:
                lt = t - st
                env = math.exp(-lt * 12.0)
                val += math.sin(2.0 * math.pi * f * lt) * env * 0.35
        samples.append(val)
    save_wav_mono("sfx_companion_spawn.wav", samples)

# 14. SFX: UI Click
def gen_sfx_ui_click():
    dur = 0.05
    num_samples = int(dur * SAMPLE_RATE)
    samples = []
    for i in range(num_samples):
        t = i / SAMPLE_RATE
        env = math.exp(-t * 80.0)
        val = math.sin(2.0 * math.pi * 1100.0 * t) * env * 0.6
        samples.append(val)
    save_wav_mono("sfx_ui_click.wav", samples)

# 15. BGM: Chill, Cheerful Background Music (Seamless loop, ~17.78 seconds, 8 bars @ 108 BPM)
def gen_bgm_cheerful():
    bpm = 108.0
    beats_per_second = bpm / 60.0
    seconds_per_beat = 1.0 / beats_per_second # ~0.5555 s
    total_bars = 8
    beats_per_bar = 4
    total_beats = total_bars * beats_per_bar # 32 beats
    total_duration = total_beats * seconds_per_beat # ~17.7777s
    num_samples = int(total_duration * SAMPLE_RATE)

    left = [0.0] * num_samples
    right = [0.0] * num_samples

    # Chord progression:
    # Bar 1-2: C Major (C - E - G)
    # Bar 3-4: G Major (G - B - D)
    # Bar 5-6: A Minor (A - C - E)
    # Bar 7-8: F Major (F - A - C)
    chords_bar = [
        ([note_freq(NOTES['C4']), note_freq(NOTES['E4']), note_freq(NOTES['G4'])], note_freq(NOTES['C3'])), # Bar 1
        ([note_freq(NOTES['C4']), note_freq(NOTES['E4']), note_freq(NOTES['G4'])], note_freq(NOTES['C3'])), # Bar 2
        ([note_freq(NOTES['G3']), note_freq(NOTES['B3']), note_freq(NOTES['D4'])], note_freq(NOTES['G3'])), # Bar 3
        ([note_freq(NOTES['G3']), note_freq(NOTES['B3']), note_freq(NOTES['D4'])], note_freq(NOTES['G3'])), # Bar 4
        ([note_freq(NOTES['A3']), note_freq(NOTES['C4']), note_freq(NOTES['E4'])], note_freq(NOTES['A3'])), # Bar 5
        ([note_freq(NOTES['A3']), note_freq(NOTES['C4']), note_freq(NOTES['E4'])], note_freq(NOTES['A3'])), # Bar 6
        ([note_freq(NOTES['F3']), note_freq(NOTES['A3']), note_freq(NOTES['C4'])], note_freq(NOTES['F3'])), # Bar 7
        ([note_freq(NOTES['F3']), note_freq(NOTES['A3']), note_freq(NOTES['C4'])], note_freq(NOTES['G3'])), # Bar 8 (resolving to G)
    ]

    # Cheerful Lead Melody (in C Major scale, relaxed & whistling/bell-like)
    # (beat_idx, duration_in_beats, note_name)
    melody = [
        # Bar 1 (C)
        (0.0, 1.0, 'E5'), (1.0, 0.5, 'G5'), (1.5, 0.5, 'A5'), (2.0, 1.0, 'G5'), (3.0, 1.0, 'E5'),
        # Bar 2 (C)
        (4.0, 0.75, 'D5'), (4.75, 0.25, 'E5'), (5.0, 1.0, 'D5'), (6.0, 2.0, 'C5'),
        # Bar 3 (G)
        (8.0, 1.0, 'D5'), (9.0, 0.5, 'E5'), (9.5, 0.5, 'G5'), (10.0, 1.0, 'B5'), (11.0, 1.0, 'A5'),
        # Bar 4 (G)
        (12.0, 1.0, 'G5'), (13.0, 1.0, 'D5'), (14.0, 2.0, 'D5'),
        # Bar 5 (Am)
        (16.0, 1.0, 'C5'), (17.0, 0.5, 'D5'), (17.5, 0.5, 'E5'), (18.0, 1.0, 'A5'), (19.0, 1.0, 'G5'),
        # Bar 6 (Am)
        (20.0, 1.0, 'E5'), (21.0, 1.0, 'C5'), (22.0, 2.0, 'E5'),
        # Bar 7 (F)
        (24.0, 1.0, 'F5'), (25.0, 0.5, 'G5'), (25.5, 0.5, 'A5'), (26.0, 1.0, 'G5'), (27.0, 1.0, 'F5'),
        # Bar 8 (F -> G cadence)
        (28.0, 1.0, 'E5'), (29.0, 1.0, 'D5'), (30.0, 2.0, 'C5'),
    ]

    def add_sound(buf, start_sec, duration_sec, gen_sample_fn, amp=1.0):
        start_i = int(start_sec * SAMPLE_RATE)
        end_i = min(num_samples, start_i + int(duration_sec * SAMPLE_RATE))
        for i in range(start_i, end_i):
            t = (i - start_i) / SAMPLE_RATE
            buf[i] += gen_sample_fn(t) * amp

    # 1. Synthesize Bassline (Warm electric bass)
    for bar_idx in range(total_bars):
        chord_notes, root_freq = chords_bar[bar_idx]
        bar_start_beat = bar_idx * 4.0
        # Beat 1 & Beat 3
        for beat_offset in [0.0, 2.0, 3.0]:
            b_time = (bar_start_beat + beat_offset) * seconds_per_beat
            bf = root_freq if beat_offset != 3.0 else root_freq * 1.5 # 5th on upbeat
            dur = seconds_per_beat * 0.85
            def bass_fn(t, freq=bf):
                env = math.exp(-t * 4.5)
                # Triangle + Sine bass
                ph = (freq * t) % 1.0
                tri = 4.0 * abs(ph - 0.5) - 1.0
                return (0.7 * math.sin(2.0 * math.pi * freq * t) + 0.3 * tri) * env
            add_sound(left, b_time, dur, bass_fn, amp=0.35)
            add_sound(right, b_time, dur, bass_fn, amp=0.35)

    # 2. Synthesize Chill Electric Piano / Acoustic Chords
    for bar_idx in range(total_bars):
        chord_notes, _ = chords_bar[bar_idx]
        bar_start_beat = bar_idx * 4.0
        # Arpeggiate lightly across the bar
        for beat in range(4):
            note = chord_notes[beat % len(chord_notes)]
            chord_time = (bar_start_beat + beat) * seconds_per_beat
            cdur = seconds_per_beat * 0.9
            def epiano_fn(t, freq=note):
                env = math.exp(-t * 3.8)
                s = 0.65 * math.sin(2.0 * math.pi * freq * t) + 0.25 * math.sin(4.0 * math.pi * freq * t) + 0.1 * math.sin(6.0 * math.pi * freq * t)
                return s * env
            # Gentle stereo spread
            pan = 0.4 if beat % 2 == 0 else 0.6
            add_sound(left, chord_time, cdur, epiano_fn, amp=0.22 * pan)
            add_sound(right, chord_time, cdur, epiano_fn, amp=0.22 * (1.0 - pan))

    # 3. Synthesize Drums (Kick on 1 & 3, Snare on 2 & 4, Hi-hat on 8ths)
    random.seed(333)
    for b in range(total_beats):
        b_time = b * seconds_per_beat
        # Kick on beat 0 and 2 of bar
        if b % 4 in (0, 2):
            def kick_fn(t):
                kf = 130.0 * math.exp(-t * 28.0) + 40.0
                return math.sin(2.0 * math.pi * kf * t) * math.exp(-t * 12.0)
            add_sound(left, b_time, 0.25, kick_fn, amp=0.45)
            add_sound(right, b_time, 0.25, kick_fn, amp=0.45)
        # Snare on beat 1 and 3 of bar (2 and 4 in 1-based)
        if b % 4 in (1, 3):
            def snare_fn(t):
                tone = math.sin(2.0 * math.pi * 185.0 * t) * math.exp(-t * 25.0) * 0.4
                noise = (random.random() * 2.0 - 1.0) * math.exp(-t * 18.0) * 0.5
                return tone + noise
            add_sound(left, b_time, 0.2, snare_fn, amp=0.25)
            add_sound(right, b_time, 0.2, snare_fn, amp=0.25)
        # Hi-hat on every 8th note
        for sub in [0.0, 0.5]:
            hh_time = (b + sub) * seconds_per_beat
            def hihat_fn(t):
                return (random.random() * 2.0 - 1.0) * math.exp(-t * 70.0)
            add_sound(left, hh_time, 0.05, hihat_fn, amp=0.08)
            add_sound(right, hh_time, 0.05, hihat_fn, amp=0.08)

    # 4. Synthesize Melodic Lead
    for m_beat, m_dur_beats, m_note in melody:
        m_time = m_beat * seconds_per_beat
        m_freq = note_freq(NOTES[m_note])
        m_dur = m_dur_beats * seconds_per_beat
        def lead_fn(t, freq=m_freq, dur=m_dur):
            # Soft attack & decay
            att = min(1.0, t / 0.03)
            rel = math.exp(-(max(0.0, t - dur)) * 8.0) if t > dur else 1.0
            env = att * rel * math.exp(-t * 1.8)
            vibrato = 1.0 + 0.005 * math.sin(2.0 * math.pi * 6.0 * t) if t > 0.15 else 1.0
            f = freq * vibrato
            # Warm flute / bell-like synth
            w = 0.7 * math.sin(2.0 * math.pi * f * t) + 0.2 * math.sin(4.0 * math.pi * f * t) + 0.1 * math.sin(6.0 * math.pi * f * t)
            return w * env
        add_sound(left, m_time, m_dur + 0.15, lead_fn, amp=0.38)
        add_sound(right, m_time, m_dur + 0.15, lead_fn, amp=0.38)

    save_wav_stereo("bgm_cheerful.wav", left, right)

def main():
    print("Synthesizing Game Sound Effects & Chill BGM...")
    gen_sfx_catch_regular()
    gen_sfx_catch_golden()
    gen_sfx_catch_grow()
    gen_sfx_catch_legendary()
    gen_sfx_splat()
    gen_sfx_poop_hit()
    gen_sfx_poop_ground()
    gen_sfx_thunder()
    gen_sfx_shock()
    gen_sfx_level_up()
    gen_sfx_game_over()
    gen_sfx_grow_use()
    gen_sfx_companion_spawn()
    gen_sfx_ui_click()
    gen_bgm_cheerful()
    print("All audio files successfully generated in 'audio/' folder!")

if __name__ == "__main__":
    main()
