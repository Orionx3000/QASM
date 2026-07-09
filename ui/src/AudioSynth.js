// AudioSynth.js
// 100% Native Web Audio API procedural synthesizer (No dependencies)

class ProceduralSynth {
  constructor() {
    this.audioCtx = null;
    this.masterGain = null;
    this.humGain = null;
    this.humOsc = null;
    this.humLFO = null;
    this.initialized = false;
    this.isTTSActive = false;
  }

  setTTSActive(active) {
    this.isTTSActive = active;
  }

  init() {
    if (this.initialized) return;
    try {
      this.audioCtx = new (window.AudioContext || window.webkitAudioContext)();
      this.masterGain = this.audioCtx.createGain();
      this.masterGain.gain.value = 0.2; // Keep volume reasonable
      this.masterGain.connect(this.audioCtx.destination);
      
      this.humGain = this.audioCtx.createGain();
      this.humGain.gain.value = 0.0; // Starts silent
      this.humGain.connect(this.masterGain);

      this.initialized = true;
      console.log("[AUDIO] Polyphonic Web Audio API Synthesizer Initialized.");
      // this.startAmbientHum(); // Disabled for now until Android APK haptics are needed
    } catch (e) {
      console.error("Web Audio API not supported:", e);
    }
  }

  // Continuous LFO-driven hum for "Gizmo Purring" / Haptics
  startAmbientHum() {
    if (!this.audioCtx) return;

    this.humOsc = this.audioCtx.createOscillator();
    this.humOsc.type = 'sine';
    this.humOsc.frequency.value = 55; // Low bass/sub frequency (A1)

    this.humLFO = this.audioCtx.createOscillator();
    this.humLFO.type = 'sine';
    this.humLFO.frequency.value = 4; // 4Hz modulation (purr speed)

    const lfoGain = this.audioCtx.createGain();
    lfoGain.gain.value = 0.5;

    this.humLFO.connect(lfoGain);
    lfoGain.connect(this.humGain.gain); // Modulate amplitude

    this.humOsc.connect(this.humGain);
    
    this.humOsc.start();
    this.humLFO.start();
    // Start completely silent, only trigger on state changes
    this.humGain.gain.setValueAtTime(0, this.audioCtx.currentTime);
  }

  // Connector ready for future APK touch/petting events
  triggerHapticPurr(intensity = 0.5, duration = 2.0) {
    if (!this.humGain || !this.humLFO) return;
    
    const now = this.audioCtx.currentTime;
    this.humLFO.frequency.setTargetAtTime(2 + (intensity * 3), now, 0.5);
    
    this.humGain.gain.cancelScheduledValues(now);
    this.humGain.gain.setValueAtTime(0, now);
    this.humGain.gain.linearRampToValueAtTime(intensity * 0.5, now + 0.1); // Quick ramp up
    this.humGain.gain.setTargetAtTime(0, now + duration, duration * 0.75); // Slow fade out
  }

  // Generate a completely unique R2-D2 style sound based on a seed or state index (0-138)
  playStateSound(stateIndex, complexityLevel = 5) {
    if (this.isTTSActive) return; // Duck sounds completely if TTS is speaking
    if (!this.initialized || !this.audioCtx) return;

    const now = this.audioCtx.currentTime;
    
    // Procedurally map the stateIndex (0-60+) to sound characteristics
    // This allows infinite completely unique audio signatures natively.
    const waveTypes = ['sine', 'triangle', 'square', 'sawtooth'];
    
    // Derive characteristics mathematically from the index to ensure reproducibility
    const baseFreq = 200 + ((stateIndex * 137) % 1800); // Freq between 200Hz and 2000Hz
    const waveType = waveTypes[(stateIndex * 3) % 4];
    const chirpCount = 1 + ((stateIndex * 7) % 6); // 1 to 6 chirps

    // Determine if state is Consonant or Dissonant mathematically
    const isConsonant = (stateIndex % 2) === 0;

    // Polyphonic Triad (Root, Third, Fifth)
    const ratios = isConsonant 
      ? [1, 1.25, 1.5] // Major Triad
      : [1, 1.189, 1.414]; // Minor/Diminished (Tense)

    const duration = 0.15 + (((stateIndex * 11) % 100) / 200); // 0.15s to 0.65s per chirp

    let timeCursor = now;

    // Build the procedural envelope and frequency sweeps (Polyphonic R2-D2 style)
    for (let i = 0; i < chirpCount; i++) {
      const freqMod = ((stateIndex * (i + 1) * 17) % 500) - 250; 
      const startFreq = baseFreq + freqMod;
      const endFreq = baseFreq + freqMod + (((stateIndex * (i + 1) * 23) % 1000) - 500);

      // Spawn 3 oscillators for polyphony
      ratios.forEach((ratio, voiceIdx) => {
        const osc = this.audioCtx.createOscillator();
        const gain = this.audioCtx.createGain();

        osc.type = waveType;
        osc.connect(gain);
        gain.connect(this.masterGain);

        osc.frequency.setValueAtTime(startFreq * ratio, timeCursor);
        osc.frequency.exponentialRampToValueAtTime(endFreq * ratio, timeCursor + duration);

        gain.gain.setValueAtTime(0, timeCursor);
        gain.gain.linearRampToValueAtTime(0.2, timeCursor + (duration * 0.2)); // Attack
        gain.gain.setValueAtTime(0.2, timeCursor + (duration * 0.8)); // Sustain
        gain.gain.linearRampToValueAtTime(0, timeCursor + duration); // Release

        osc.start(timeCursor);
        osc.stop(timeCursor + duration);
      });

      timeCursor += duration + 0.05; // Gap between chirps
    }
  }
}

export const synth = new ProceduralSynth();
