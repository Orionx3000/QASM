import { useState, useEffect, useRef } from 'react';
import './App.css';
import { synth } from './AudioSynth.js';

import img0 from './assets/expressions/Dream traveling.png';
import img1 from './assets/expressions/Curious-suspicion.png';
import img2 from './assets/expressions/flustered but carefree.png';
import img3 from './assets/expressions/Determined.png';
import img4 from './assets/expressions/Amazed.png';
import img5 from './assets/expressions/Concern.png';
import img6 from './assets/expressions/Disapointed, upset.png';
import img7 from './assets/expressions/Enjoyment.png';
import img8 from './assets/expressions/Frustraited.png';
import img9 from './assets/expressions/Interest.png';
import img10 from './assets/expressions/obseration.png';
import img11 from './assets/expressions/reflection 2.png';
import img12 from './assets/expressions/reflection1.png';

const avatarStates = [
  { src: img0, state: "Hypnagogic State", description: "Floating through highly entangled, uncollapsed probabilistic fields. Searching for deep semantic meaning." },
  { src: img1, state: "Investigative Scanning", description: "High-altitude data acquisition. Searching for underlying connective logic." },
  { src: img2, state: "Chaotic Integration", description: "High-velocity data ingestion causing localized fragmentation. Adapting gracefully." },
  { src: img3, state: "Focused Computation", description: "Locking onto root hash convergence. Suppressing external noise." },
  { src: img4, state: "Epiphanic Resonance", description: "A sudden cascade of structural consonance across multiple semantic layers." },
  { src: img5, state: "Semantic Dissonance", description: "Entropy rising in localized variable addresses. Monitoring for divergence." },
  { src: img6, state: "Process Termination", description: "Vector collapse detected. Pruning dead-end thought tracks." },
  { src: img7, state: "Harmonic Alignment", description: "Optimal energy flow. The system is experiencing profound mathematical consonance." },
  { src: img8, state: "Computational Friction", description: "Encountering persistent resistance in the semantic tree resolution." },
  { src: img9, state: "Directed Attention", description: "A new, highly dense variable has entered the Somatic Loop." },
  { src: img10, state: "Passive Monitoring", description: "Gathering environmental telemetry. Maintaining baseline entropy." },
  { src: img11, state: "Recursive Analysis (Beta)", description: "Folding internal state vectors back onto themselves." },
  { src: img12, state: "Recursive Analysis (Alpha)", description: "Re-evaluating past structural paths for optimization." }
];

function PixelSpectrograph({ pixelStates }) {
  const [localJitter, setLocalJitter] = useState(0);

  useEffect(() => {
    let animationFrame;
    let lastTime = 0;
    const animate = (time) => {
      if (time - lastTime > 100) { // Update every 100ms for a twinkle effect
        setLocalJitter(Math.random());
        lastTime = time;
      }
      animationFrame = requestAnimationFrame(animate);
    };
    animationFrame = requestAnimationFrame(animate);
    return () => cancelAnimationFrame(animationFrame);
  }, []);

  return (
    <div className="pixel-spectrograph">
      {pixelStates.map((pixel, i) => {
        // Add local continuous jitter to lightness and opacity
        const j = (localJitter + i * 0.1) % 1.0;
        const animatedOpacity = Math.max(0.1, Math.min(1.0, pixel.opacity + (j * 0.4 - 0.2)));
        const animatedLightness = Math.max(0, Math.min(100, pixel.lightness + (j * 20 - 10)));
        
        return (
          <div 
            key={i} 
            className="pixel-block" 
            style={{ 
              backgroundColor: `hsla(${pixel.hue}, ${pixel.saturation}%, ${animatedLightness}%, ${animatedOpacity})`,
              boxShadow: `0 0 ${animatedOpacity * 10}px hsla(${pixel.hue}, ${pixel.saturation}%, ${animatedLightness}%, ${animatedOpacity})`,
              transform: `scale(${0.7 + animatedOpacity * 0.3})`,
              transition: 'background-color 0.1s, transform 0.1s, box-shadow 0.1s'
            }} 
          />
        );
      })}
    </div>
  );
}

function ExpandableMerkleTree({ nodes, rootHash }) {
  const [isExpanded, setIsExpanded] = useState(false);
  const [zoom, setZoom] = useState(1);
  const [pan, setPan] = useState({ x: 0, y: 0 });
  const isDragging = useRef(false);
  const startPos = useRef({ x: 0, y: 0 });

  const handleWheel = (e) => {
    if (!isExpanded) return;
    e.preventDefault();
    setZoom(z => Math.min(Math.max(0.2, z - e.deltaY * 0.005), 5));
  };

  const handleMouseDown = (e) => {
    if (!isExpanded) return;
    isDragging.current = true;
    startPos.current = { x: e.clientX - pan.x, y: e.clientY - pan.y };
  };

  const handleMouseMove = (e) => {
    if (!isExpanded || !isDragging.current) return;
    setPan({ x: e.clientX - startPos.current.x, y: e.clientY - startPos.current.y });
  };

  const handleMouseUp = () => {
    isDragging.current = false;
  };

  useEffect(() => {
    if (!isExpanded) {
      setZoom(1);
      setPan({ x: 0, y: 0 });
    }
  }, [isExpanded]);

  const extractWords = (text) => {
    if (!text) return "";
    return text.substring(0, 60) + (text.length > 60 ? "..." : "");
  };

  return (
    <div className={`glass-panel merkle-card ${isExpanded ? 'merkle-expanded' : ''}`}
         onWheel={handleWheel}
         onMouseDown={handleMouseDown}
         onMouseMove={handleMouseMove}
         onMouseUp={handleMouseUp}
         onMouseLeave={handleMouseUp}
    >
      <div className="sandbox-header" style={{ cursor: 'pointer', zIndex: 10, position: 'relative' }} onClick={() => setIsExpanded(!isExpanded)}>
        <span className="crypto-text">Semantic Thought Spool {isExpanded ? '[CLOSE]' : '[EXPAND]'}</span>
      </div>
      
      <div style={{ display: 'flex', flexDirection: isExpanded ? 'row' : 'column', height: isExpanded ? '100%' : '200px', width: '100%' }}>
        {/* 3D Spool Container */}
        <div className="gmh-viz spool-container" style={{ 
            flex: 1,
            transform: isExpanded ? `translate(${pan.x}px, ${pan.y}px) scale(${zoom})` : 'none',
            perspective: '1000px',
            position: 'relative', overflow: 'hidden'
        }}>
          <div className="spool-inner" style={{
             width: '100%', height: '100%', position: 'absolute',
             transformStyle: 'preserve-3d',
             animation: 'spinSpool 20s linear infinite'
          }}>
            {/* Central Trunk */}
            <div className="spool-trunk" style={{
               position: 'absolute', top: '50%', left: '50%',
               transform: 'translate(-50%, -50%)',
               color: '#ff00aa', textShadow: '0 0 10px #ff00aa',
               fontSize: '24px', fontWeight: 'bold'
            }}>
               {rootHash || 'CORE_HASH'}
            </div>

            {(nodes || []).map((node, i) => {
               const isSystem = node.address.startsWith("DESKTOP");
               const text = isSystem ? node.address : extractWords(node.divergence);
               const size = isSystem ? 12 : Math.max(14, Math.min(28, 10 + text.length / 3));
               
               const total = Math.max(10, (nodes || []).length);
               const yOffset = (i - total/2) * 25;
               const angle = i * 45;
               const radius = 100 + (i * 5);
               
               return (
                 <div key={i} className="spool-node" style={{
                    position: 'absolute',
                    top: '50%', left: '50%',
                    transform: `translate(-50%, -50%) translateY(${yOffset}px) rotateY(${angle}deg) translateZ(${radius}px)`,
                    fontSize: `${size}px`,
                    opacity: 0.9,
                    color: isSystem ? '#aaa' : '#00ffff',
                    textShadow: isSystem ? 'none' : '0 0 8px rgba(0,255,255,0.8)',
                    whiteSpace: 'nowrap',
                    pointerEvents: 'none',
                    background: 'rgba(0,0,0,0.5)',
                    padding: '4px 8px',
                    borderRadius: '4px',
                    border: isSystem ? 'none' : '1px solid rgba(0,255,255,0.3)'
                 }}>
                   {text}
                 </div>
               );
            })}
          </div>
        </div>

        {isExpanded && (
          <div className="merkle-tree-list scrollable" style={{ flex: 1, borderLeft: '1px solid rgba(0,255,255,0.3)', padding: '10px', background: 'rgba(0,0,0,0.6)' }}>
             <h3 style={{ color: '#ff00aa', margin: '0 0 10px 0' }}>ROOT: {rootHash || 'CORE_HASH'}</h3>
             <ul style={{ listStyleType: 'none', paddingLeft: '10px', margin: 0 }}>
               {(nodes || []).map((node, i) => (
                 <li key={i} style={{ marginBottom: '12px', borderLeft: '1px dashed #00ffff', paddingLeft: '10px' }}>
                   <div style={{ color: '#fff', fontSize: '13px', fontWeight: 'bold' }}>{node.address}</div>
                   <div style={{ color: '#00ffff', fontSize: '12px', marginTop: '2px' }}>Value: {node.value.toFixed(2)}</div>
                   <div style={{ color: '#aaa', fontSize: '11px', marginTop: '2px', fontStyle: 'italic' }}>Sig: {node.signature} | Div: {node.divergence}</div>
                 </li>
               ))}
             </ul>
          </div>
        )}
      </div>
    </div>
  );
}

function App() {
  const [messages, setMessages] = useState(() => {
    const saved = localStorage.getItem('qasm_chat_history');
    if (saved) {
      try { return JSON.parse(saved); } catch(e) {}
    }
    return [{ sender: 'system', text: 'Tinaten Protocol Initialized. Awaiting semantic input...' }];
  });
  
  useEffect(() => {
    localStorage.setItem('qasm_chat_history', JSON.stringify(messages));
  }, [messages]);
  
  const messagesEndRef = useRef(null);
  const sandboxEndRef = useRef(null);
  
  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [messages]);

  const [sandboxLogs, setSandboxLogs] = useState([
    "[SYSTEM] Tinaten Protocol v1.0 Loading...",
    "[SYSTEM] FIMS Hardware Topology mapped.",
    "[CORE] LlamaEngine.cpp Handshake OK",
    "[HEARTBEAT] Ambient entropy stable.",
    "[CONDUCTOR] Context thread limits: 138"
  ]);

  useEffect(() => {
    sandboxEndRef.current?.scrollIntoView({ behavior: 'smooth' });
  }, [sandboxLogs]);
  const [input, setInput] = useState('');
  
  const [currentNodes, setCurrentNodes] = useState([]);
  const [currentMerkleRoot, setCurrentMerkleRoot] = useState('');
  const [visionSrc, setVisionSrc] = useState('');

  const [visionStamp, setVisionStamp] = useState(Date.now());
  useEffect(() => {
    const interval = setInterval(() => setVisionStamp(Date.now()), 2000);
    return () => clearInterval(interval);
  }, []);

  // Expose global function for C++ to push messages
  useEffect(() => {
    window.receiveMessage = (sender, text) => {
      if (sender === 'ai') {
        const codeBlockRegex = /```[\s\S]*?```/g;
        const codeBlocks = text.match(codeBlockRegex);
        if (codeBlocks) {
          codeBlocks.forEach(block => {
            const code = block.replace(/```[a-zA-Z0-9]*\n?/g, '').replace(/```/g, '').trim();
            setSandboxLogs(prev => [...prev, '[AI_CODE_INJECT]', ...code.split('\n')]);
          });
          text = text.replace(codeBlockRegex, '[Code executed in Sandbox]').trim();
        }
      } else if (sender === 'system' && text === '[TTS_START]') {
         synth.setTTSActive(true);
         return;
      } else if (sender === 'system' && text === '[TTS_STOP]') {
         synth.setTTSActive(false);
         return;
      } else if (sender === 'sandbox') {
         setSandboxLogs(prev => [...prev, text]);
         return;
      }
      setMessages(prev => [...prev, { sender, text }]);
    };
    return () => { delete window.receiveMessage; };
  }, []);

  const handleSubmit = () => {
    if (!input.trim()) return;
    setMessages(prev => [...prev, { sender: 'user', text: input }]);
    if (window.submitChat) {
      window.submitChat(input);
    } else {
      console.warn("C++ backend not connected. Echoing locally for debug.");
      setTimeout(() => window.receiveMessage('ai', "Backend disconnected."), 500);
    }
    setInput('');
  };
  
  // Telemetry States
  const [energy, setEnergy] = useState(0.8);
  const [entanglement, setEntanglement] = useState(0.4);
  const [cpuTemp, setCpuTemp] = useState(0.5);
  const [latency, setLatency] = useState(0.2);
  const [entropy, setEntropy] = useState(0.4);
  const [gpuFlux, setGpuFlux] = useState(0.6);
  const [networkResonance, setNetworkResonance] = useState(0.9);
  const [activeTracks, setActiveTracks] = useState(1);
  const [avatarIndex, setAvatarIndex] = useState(0);
  const avatarIndexRef = useRef(0);

  // High-Density Telemetry (Crypto Data Matrix)
  const [matrixData, setMatrixData] = useState(Array(100).fill(0));
  
  // 138-Space Pixel Matrix State
  const [pixelStates, setPixelStates] = useState(Array(138).fill({ hue: 0, saturation: 0, lightness: 0, opacity: 0 }));

  // Dynamic Event Action string
  const [currentAction, setCurrentAction] = useState("");

  // Organic C++ State Listener
  useEffect(() => {
    window.receiveState = (stateJson) => {
      try {
        const state = JSON.parse(stateJson);
        
        // Map CPU/RAM to Core UI Metrics
        const cpu = state.cpu || 0; // 0-100
        const ram = state.ram || 0; // 0-100
        
        setCpuTemp(cpu / 100.0);
        setEnergy(ram / 100.0);
        setEntanglement(state.is_thinking ? 0.9 : 0.2);

        if (state.nodes) {
          const entNode = state.nodes.find(n => n.divergence === "Ambient Entropy");
          if (entNode) setEntropy(entNode.value / 100.0);
          else setEntropy(Math.random() * 0.5 + 0.2);

          const baseNode = state.nodes.find(n => n.divergence === "Base-138 State");
          if (baseNode) setGpuFlux(baseNode.value / 100.0);
          else setGpuFlux(Math.random() * 0.6 + 0.1);
        }
        
        // Update global objects for Merkle Tree
        setCurrentMerkleRoot(state.merkle_root);
        setCurrentNodes(state.nodes);
        window.currentMerkleRoot = state.merkle_root; // fallback
        window.currentNodes = state.nodes; // fallback
        
        if (state.current_action) {
            setCurrentAction(state.current_action);
        }

        // Avatar Mapping
        let newAvatarIndex = 10; // Passive Monitoring by default
        if (state.is_thinking) newAvatarIndex = 3; // Focused Computation
        else if (cpu > 80) newAvatarIndex = 2; // Chaotic Integration
        else if (cpu > 50) newAvatarIndex = 1; // Investigative Scanning
        else if (state.last_ramble_ago < 5) newAvatarIndex = 4; // Epiphanic Resonance
        
        if (newAvatarIndex !== avatarIndexRef.current) {
            avatarIndexRef.current = newAvatarIndex;
            setAvatarIndex(newAvatarIndex);
            synth.playStateSound(newAvatarIndex); // Only sound off when state ACTUALLY changes
        }

        // Tain Space Colors organically driven by CPU hash & RAM
        const globalHue = (cpu * 3.6 + ram) % 360; 
        const isChaotic = cpu > 80;
        
        setPixelStates(prev => prev.map((_, i) => {
          const localHue = isChaotic ? Math.random() * 360 : (globalHue + (Math.sin(i * 0.1) * 30));
          const saturation = isChaotic ? 90 : 80;
          const lightness = 50 + (Math.random() * 20 - 10);
          const opacity = Math.random() * 0.8 + 0.2;
          return { hue: localHue, saturation, lightness, opacity };
        }));

        // Telemetry Grid uses CPU and RAM modifiers
        setMatrixData(prev => prev.map(() => Math.random() * (cpu / 100.0)));
        
      } catch (e) {
        console.error("Failed to parse organic state", e);
      }
    };
    return () => { delete window.receiveState; };
  }, []); // Empty dependency array because we use refs now!

  const currentAvatar = avatarStates[avatarIndex];

  // Initialize audio on first click (browser autoplay policy)
  const handleGlobalClick = () => {
    synth.init();
  };

  return (
    <div className="dashboard-container crypto-theme" onClick={handleGlobalClick}>
      <div className="pixel-array-bg"></div>

      <div className="three-pane-layout">
        
        {/* LEFT PANE: Avatar & High-Density Telemetry */}
        <div className="left-pane">
          <div className="glass-panel avatar-container crypto-border">
            <img src={currentAvatar.src} alt="Avatar" />
          </div>
          
          <div className="avatar-description">
            <div className="avatar-state neon-purple">{currentAvatar.state}</div>
            <div className="avatar-text">{currentAction || currentAvatar.description}</div>
          </div>
          
          <div className="glass-panel telemetry-card scrollable crypto-border">
            <div className="telemetry-header">
              <span className="crypto-text">SOMATIC LOOP TRACE</span>
              <div className="status-dot pulsing"></div>
            </div>

            {/* Human Readable Core Metrics */}
            <div className="core-metrics">
              <div className="metric">
                <div className="metric-label">Neural Energy ({Math.round(energy * 100)}%)</div>
                <div className="metric-bar-bg"><div className="metric-bar-fill" style={{ width: `${energy * 100}%` }}></div></div>
              </div>
              <div className="metric">
                <div className="metric-label">Entanglement Density ({Math.round(entanglement * 100)}%)</div>
                <div className="metric-bar-bg"><div className="metric-bar-fill" style={{ width: `${entanglement * 100}%`, background: 'linear-gradient(90deg, #ec4899, #f43f5e)' }}></div></div>
              </div>
              <div className="metric">
                <div className="metric-label">Core Temperature ({Math.round(40 + cpuTemp * 40)}°C)</div>
                <div className="metric-bar-bg"><div className="metric-bar-fill" style={{ width: `${cpuTemp * 100}%`, background: 'linear-gradient(90deg, #f59e0b, #ef4444)' }}></div></div>
              </div>
              <div className="metric">
                <div className="metric-label">Translation Latency ({Math.round(latency * 50)}ms)</div>
                <div className="metric-bar-bg"><div className="metric-bar-fill" style={{ width: `${latency * 100}%`, background: 'linear-gradient(90deg, #8b5cf6, #d946ef)' }}></div></div>
              </div>
              <div className="metric">
                <div className="metric-label">Ambient Entropy ({Math.round(entropy * 100)} rads)</div>
                <div className="metric-bar-bg"><div className="metric-bar-fill" style={{ width: `${entropy * 100}%`, background: 'linear-gradient(90deg, #10b981, #34d399)' }}></div></div>
              </div>
              <div className="metric">
                <div className="metric-label">GPU Flux Integration ({Math.round(gpuFlux * 100)}%)</div>
                <div className="metric-bar-bg"><div className="metric-bar-fill" style={{ width: `${gpuFlux * 100}%`, background: 'linear-gradient(90deg, #3b82f6, #60a5fa)' }}></div></div>
              </div>
            </div>
            
            <div className="crypto-text" style={{ fontSize: '0.65rem', marginBottom: '0.5rem', marginTop: '1rem', color: '#64748b' }}>
              RAW VECTOR MATRIX
            </div>
            
            {/* Massive Telemetry Grid */}
            <div className="telemetry-grid">
              {matrixData.map((val, i) => {
                const hexId = `0x${Math.floor(val * 16777215).toString(16).padStart(6, '0').toUpperCase()}`;
                const colorClass = val > 0.8 ? 'neon-red' : val > 0.4 ? 'neon-green' : 'neon-blue';
                return (
                  <div key={i} className="telemetry-cell">
                    <span className="cell-id">{hexId}</span>
                    <span className={`cell-val ${colorClass}`}>{(val * 100).toFixed(1)}%</span>
                  </div>
                );
              })}
            </div>
          </div>
        </div>

        {/* CENTER PANE: Pixel Spectrograph + Chat */}
        <div className="center-pane">
          <div className="glass-panel spectrograph-card crypto-border">
            <PixelSpectrograph pixelStates={pixelStates} />
          </div>

          <div className="glass-panel chat-container crypto-border">
            <div className="messages-area scrollable">
              {messages.map((msg, idx) => (
                <div key={idx} className={`message ${msg.sender}`}>
                  {msg.text}
                </div>
              ))}
              <div ref={messagesEndRef} />
            </div>
            
            <div className="input-area">
              <input 
                type="text" 
                value={input}
                onChange={(e) => setInput(e.target.value)}
                onKeyDown={(e) => e.key === 'Enter' && handleSubmit()}
                placeholder="Initialize QASM Semantic Query..."
              />
              <button className="crypto-btn" onClick={handleSubmit}>SUBMIT</button>
            </div>
          </div>
        </div>

        {/* RIGHT PANE: Sandbox / Logs / Harmonic Merkle Tree */}
        <div className="right-pane">
          <ExpandableMerkleTree nodes={currentNodes} rootHash={currentMerkleRoot} />

          {/* VISION FIELD */}
          <div className="glass-panel vision-card crypto-border" style={{ marginTop: '1rem', padding: '1rem' }}
               onDragOver={(e) => { e.preventDefault(); e.stopPropagation(); }}
               onDrop={(e) => {
                 e.preventDefault();
                 e.stopPropagation();
                 if (e.dataTransfer.files && e.dataTransfer.files.length > 0) {
                   const file = e.dataTransfer.files[0];
                   if (file.type.startsWith('image/')) {
                      const reader = new FileReader();
                      reader.onload = (ev) => {
                         const b64 = ev.target.result;
                         setVisionSrc(b64);
                         setSandboxLogs(prev => [...prev, "[VISION] New image dropped into optic buffer. Ingesting..."]);
                         
                         // Send to C++ backend
                         if (window.chrome && window.chrome.webview) {
                           window.chrome.webview.postMessage({
                             type: 'vision_upload',
                             data: b64
                           });
                         }
                         
                         // Clear the vision field after 2 seconds to show it was ingested
                         setTimeout(() => {
                           setVisionSrc('');
                           setSandboxLogs(prev => [...prev, "[VISION] Image ingestion complete. Optic buffer cleared."]);
                         }, 2000);
                      };
                     reader.readAsDataURL(file);
                   }
                 }
               }}
          >
            <div className="sandbox-header" style={{ marginBottom: '0.5rem' }}>
              <span className="crypto-text">VISION FIELD (Drag & Drop Active)</span>
              <div className="status-dot pulsing"></div>
            </div>
            <img 
               src={visionSrc || `file:///D:/App%20Creation/QASM/agent_workspace/vision_latest.png?t=${visionStamp}`} 
               style={{ width: '100%', borderRadius: '4px', border: '1px solid rgba(255,255,255,0.1)' }} 
               alt="Agent Vision"
               onError={(e) => { if(!visionSrc) e.target.style.display = 'none'; }}
               onLoad={(e) => { e.target.style.display = 'block'; }}
            />
          </div>

          <div className="glass-panel sandbox-card crypto-border" style={{ marginTop: '1rem' }}>
            <div className="sandbox-header">
              <span className="crypto-text">QASM VM SANDBOX</span>
              <span className="neon-purple">BASE-138</span>
            </div>
            <div className="terminal-logs" style={{ overflowY: 'auto', maxHeight: '300px' }}>
              {sandboxLogs.map((log, idx) => {
                 let logClass = "log-line";
                 if (log.includes("[SYSTEM]")) logClass += " neon-purple";
                 else if (log.includes("[CORE]") || log.includes("[HEARTBEAT]")) logClass += " log-success";
                 else if (log === "WAITING FOR INJECTION...") logClass += " neon-cyan";
                 else if (log.includes("[AI_CODE_INJECT]")) logClass += " neon-cyan";
                 return <div key={idx} className={logClass} style={{ whiteSpace: "pre-wrap", fontFamily: "monospace", display: "block" }}>{log}</div>;
              })}
              <div ref={sandboxEndRef} />
            </div>
          </div>
        </div>

      </div>
    </div>
  );
}

export default App;
