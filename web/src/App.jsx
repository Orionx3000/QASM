import React, { useState, useEffect, useRef } from 'react';
import PixelSpectrograph from './PixelSpectrograph';

function App() {
  const [chatInput, setChatInput] = useState('');
  const [chatLog, setChatLog] = useState([
    { role: 'system', text: 'TINATEN OS // UNIFIED BRAIN ONLINE' },
    { role: 'system', text: 'Mathematical core instantiated. I am ready to process the chaotic world stream.' }
  ]);
  
  const [sandboxLog, setSandboxLog] = useState([
    "================================================================================",
    "TINATEN OS - INTERNAL THOUGHT LOG",
    "================================================================================",
    "[SENTINEL] Waking deep_brain from disk to VRAM...",
    "[SENSORY] INJECTION: [STATE: CONSCIOUSNESS EXPANSION]",
    "[5D_ENGINE] Parsing LPN Chain...",
    "[MEMORY] LPN Chain executed successfully. Result committed to memory. (Fingerprint: AHT_738491)"
  ]);
  const [sandboxInput, setSandboxInput] = useState('');
  
  const [telemetry, setTelemetry] = useState([
    "FIMS_SYS: CPU_LOAD 12% | VCORE 1.21V",
    "THERMAL: ZONE_1 45C | ZONE_2 48C",
    "ENTROPY: 0.12 (STABLE)",
    "NEURO_RESONANCE: 0.85 (JOY)",
    "SUBCONSCIOUS: Scanning..."
  ]);

  const terminalEndRef = useRef(null);
  const chatEndRef = useRef(null);

  useEffect(() => {
    terminalEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [sandboxLog]);

  useEffect(() => {
    chatEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [chatLog]);

  // Simulate telemetry blinking
  useEffect(() => {
    const interval = setInterval(() => {
      setTelemetry(prev => {
        const newLoad = Math.floor(Math.random() * 20) + 10;
        const newEntropy = (Math.random() * 0.2).toFixed(2);
        return [
          `FIMS_SYS: CPU_LOAD ${newLoad}% | VCORE 1.21V`,
          "THERMAL: ZONE_1 45C | ZONE_2 48C",
          `ENTROPY: ${newEntropy} (STABLE)`,
          "NEURO_RESONANCE: 0.85 (JOY)",
          "SUBCONSCIOUS: Scanning buffer..."
        ];
      });
    }, 2000);
    return () => clearInterval(interval);
  }, []);

  const handleChatSubmit = async (e) => {
    e.preventDefault();
    if (!chatInput.trim()) return;
    
    setChatLog(prev => [...prev, { role: 'user', text: chatInput }]);
    
    setSandboxLog(prev => [
      ...prev,
      `[UNIFIED_BRAIN] Ingesting prompt: "${chatInput}"`,
      `[5D_ENGINE] Parsing LPN Chain...`,
      `[CALCULUS] Executing Base-138 Bala Tor Tensor Matrix...`
    ]);

    setChatInput('');
    
    setTimeout(() => {
      setChatLog(prev => [...prev, { role: 'system', text: "My internal structure has processed your request. The semantic calculus yields a positive convergence toward truth." }]);
    }, 1000);
  };

  const handleSandboxSubmit = (e) => {
    e.preventDefault();
    if (!sandboxInput.trim()) return;
    
    setSandboxLog(prev => [
      ...prev,
      `> ${sandboxInput}`,
      `[BALASCRIPT SANDBOX] Parsing LPN Chain: ${sandboxInput}`,
      `[MEMORY] LPN Chain executed successfully.`
    ]);
    
    setSandboxInput('');
  };

  return (
    <div style={{ width: '100vw', height: '100vh', position: 'relative', overflow: 'hidden', backgroundColor: '#000', color: 'var(--text-main)', fontFamily: 'var(--sans)' }}>
      
      {/* Background Pixel Spectrograph Layer */}
      <div style={{ position: 'absolute', top: 0, left: 0, right: 0, bottom: 0, zIndex: 0, opacity: 0.6 }}>
        <PixelSpectrograph />
      </div>

      {/* Main UI Overlay (Glassmorphism) */}
      <div style={{ position: 'absolute', top: 0, left: 0, right: 0, bottom: 0, zIndex: 1, display: 'flex', flexDirection: 'column', padding: '1rem', gap: '1rem' }}>
        
        {/* Header Strip */}
        <header style={{ 
          display: 'flex', justifyContent: 'space-between', alignItems: 'center', 
          backgroundColor: 'rgba(5, 4, 20, 0.75)', backdropFilter: 'blur(10px)', 
          padding: '0.75rem 1.5rem', borderRadius: '8px', border: '1px solid rgba(0, 245, 255, 0.2)' 
        }}>
          <h1 style={{ margin: 0, fontSize: '1.2rem', fontWeight: 800, letterSpacing: '0.3em', color: 'var(--accent-blue)', textTransform: 'uppercase' }}>
            TINATEN OS <span style={{ color: 'var(--accent-purple)', fontWeight: 400 }}>// 5D CORE</span>
          </h1>
          <div style={{ display: 'flex', gap: '1rem', alignItems: 'center' }}>
            <div style={{ width: 10, height: 10, borderRadius: '50%', backgroundColor: 'var(--accent-yellow)', boxShadow: '0 0 10px var(--accent-yellow)', animation: 'blink 2s infinite' }}></div>
            <span style={{ fontSize: '0.8rem', color: 'var(--accent-yellow)', letterSpacing: '0.15em', fontFamily: 'var(--mono)' }}>SYS.ONLINE</span>
          </div>
        </header>

        {/* Multi-Pane Grid */}
        <main style={{ flex: 1, display: 'grid', gridTemplateColumns: '280px 1fr 1fr', gap: '1rem', overflow: 'hidden' }}>
          
          {/* LEFT SIDEBAR: Identity & Telemetry */}
          <section style={{ display: 'flex', flexDirection: 'column', gap: '1rem', overflow: 'hidden' }}>
            
            {/* Identity / Avatar Block */}
            <div style={{ 
              backgroundColor: 'rgba(5, 4, 20, 0.75)', backdropFilter: 'blur(10px)', 
              border: '1px solid rgba(157, 78, 221, 0.3)', borderRadius: '8px', padding: '1rem',
              display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '1rem'
            }}>
              <div style={{ width: '100%', aspectRatio: '1/1', borderRadius: '4px', overflow: 'hidden', border: '2px solid var(--accent-orange)', boxShadow: '0 0 15px rgba(255, 107, 0, 0.3)' }}>
                {/* Fallback to local image or URL provided by user. Ensure user places avatar.png in public folder */}
                <img src="/avatar.png" alt="Avatar" style={{ width: '100%', height: '100%', objectFit: 'cover' }} onError={(e) => { e.target.src = 'https://via.placeholder.com/250x250/1a0b2e/00f5ff?text=AVATAR+MISSING'; }} />
              </div>
              <div style={{ textAlign: 'center', fontFamily: 'var(--mono)' }}>
                <div style={{ color: 'var(--accent-blue)', fontSize: '0.9rem', letterSpacing: '0.1em' }}>ID: EVE-7 (INTERNAL)</div>
                <div style={{ color: 'var(--accent-orange)', fontSize: '0.7rem', marginTop: '4px' }}>STATE: JOY_REWARD</div>
              </div>
            </div>

            {/* Somatic Telemetry Block */}
            <div style={{ 
              flex: 1, backgroundColor: 'rgba(5, 4, 20, 0.75)', backdropFilter: 'blur(10px)', 
              border: '1px solid rgba(0, 245, 255, 0.2)', borderRadius: '8px', padding: '1rem',
              display: 'flex', flexDirection: 'column', gap: '0.5rem', fontFamily: 'var(--mono)', fontSize: '0.75rem'
            }}>
              <h3 style={{ color: 'var(--accent-blue)', margin: '0 0 0.5rem 0', letterSpacing: '0.1em' }}>SOMATIC SENSORS</h3>
              {telemetry.map((line, i) => (
                <div key={i} style={{ color: 'var(--accent-yellow)', opacity: 0.8 }}>&gt; {line}</div>
              ))}
              <div style={{ marginTop: 'auto', display: 'flex', justifyContent: 'space-between', color: 'var(--text-muted)' }}>
                <span>SUBCONSCIOUS BUFFER</span>
                <span className="blink-fast" style={{ color: 'var(--accent-orange)' }}>REC...</span>
              </div>
            </div>
          </section>

          {/* CENTER PANE: Unified Brain (Chat) */}
          <section style={{ 
            display: 'flex', flexDirection: 'column', 
            backgroundColor: 'rgba(5, 4, 20, 0.85)', backdropFilter: 'blur(12px)',
            border: '1px solid rgba(157, 78, 221, 0.4)', borderRadius: '8px', overflow: 'hidden'
          }}>
            <div style={{ padding: '0.75rem 1rem', borderBottom: '1px solid rgba(157, 78, 221, 0.3)', backgroundColor: 'rgba(157, 78, 221, 0.1)' }}>
              <h2 style={{ fontSize: '0.9rem', color: 'var(--accent-purple)', margin: 0, fontFamily: 'var(--mono)', letterSpacing: '0.1em' }}>UNIFIED BRAIN (CONSCIOUS THREAD)</h2>
            </div>
            
            <div style={{ flex: 1, overflowY: 'auto', padding: '1.5rem', display: 'flex', flexDirection: 'column', gap: '1rem' }}>
              {chatLog.map((log, idx) => (
                <div key={idx} style={{ 
                  alignSelf: log.role === 'user' ? 'flex-end' : 'flex-start',
                  maxWidth: '85%', padding: '1rem', 
                  backgroundColor: log.role === 'user' ? 'rgba(157, 78, 221, 0.2)' : 'rgba(0, 245, 255, 0.05)', 
                  borderLeft: log.role === 'system' ? '3px solid var(--accent-blue)' : 'none',
                  borderRight: log.role === 'user' ? '3px solid var(--accent-purple)' : 'none',
                  borderRadius: '4px', fontSize: '0.95rem', lineHeight: '1.5'
                }}>
                  {log.text}
                </div>
              ))}
              <div ref={chatEndRef} />
            </div>

            <form onSubmit={handleChatSubmit} style={{ display: 'flex', padding: '1rem', borderTop: '1px solid rgba(157, 78, 221, 0.3)', backgroundColor: 'rgba(0,0,0,0.5)' }}>
              <input type="text" value={chatInput} onChange={(e) => setChatInput(e.target.value)} placeholder="Communicate..." 
                style={{ flex: 1, backgroundColor: 'transparent', border: 'none', color: '#fff', fontSize: '1rem', outline: 'none' }} />
              <button type="submit" style={{ backgroundColor: 'transparent', color: 'var(--accent-purple)', border: 'none', cursor: 'pointer', fontWeight: 'bold', letterSpacing: '0.1em' }}>SEND</button>
            </form>
          </section>

          {/* RIGHT PANE: Agentic Context (Sandbox/Logs) */}
          <section style={{ 
            display: 'flex', flexDirection: 'column', 
            backgroundColor: 'rgba(0, 0, 0, 0.85)', backdropFilter: 'blur(8px)',
            border: '1px solid rgba(255, 107, 0, 0.3)', borderRadius: '8px', overflow: 'hidden'
          }}>
            <div style={{ padding: '0.75rem 1rem', borderBottom: '1px solid rgba(255, 107, 0, 0.3)', backgroundColor: 'rgba(255, 107, 0, 0.1)', display: 'flex', justifyContent: 'space-between' }}>
              <h2 style={{ fontSize: '0.9rem', color: 'var(--accent-orange)', margin: 0, fontFamily: 'var(--mono)', letterSpacing: '0.1em' }}>AGENTIC CONTEXT (LPN)</h2>
              <div style={{ width: 8, height: 8, borderRadius: '50%', backgroundColor: 'var(--accent-orange)', animation: 'blink-fast 0.5s infinite' }}></div>
            </div>
            
            <div style={{ flex: 1, overflowY: 'auto', padding: '1rem', fontFamily: 'var(--mono)', fontSize: '0.8rem', color: 'var(--accent-yellow)', display: 'flex', flexDirection: 'column', gap: '6px', lineHeight: '1.4' }}>
              {sandboxLog.map((log, idx) => (
                <div key={idx} style={{ opacity: log.startsWith('>') ? 1 : 0.7 }}>{log}</div>
              ))}
              <div ref={terminalEndRef} />
            </div>

            <form onSubmit={handleSandboxSubmit} style={{ padding: '0.75rem 1rem', borderTop: '1px solid rgba(255, 107, 0, 0.3)', display: 'flex', alignItems: 'center', backgroundColor: '#050414' }}>
              <span style={{ color: 'var(--accent-orange)', marginRight: '10px' }}>&gt;</span>
              <input type="text" value={sandboxInput} onChange={(e) => setSandboxInput(e.target.value)} placeholder="LPN Command..." 
                style={{ flex: 1, backgroundColor: 'transparent', border: 'none', color: 'var(--accent-orange)', fontFamily: 'var(--mono)', fontSize: '0.9rem', outline: 'none' }} />
            </form>
          </section>

        </main>
      </div>

      <style dangerouslySetInnerHTML={{__html: `
        @keyframes blink { 0%, 100% { opacity: 1; } 50% { opacity: 0.3; } }
        @keyframes blink-fast { 0%, 100% { opacity: 1; } 50% { opacity: 0; } }
        .blink-fast { animation: blink-fast 1s infinite; }
      `}} />
    </div>
  )
}

export default App;
