import React, { useRef, useEffect, useState } from 'react';

export default function PixelSpectrograph() {
  const canvasRef = useRef(null);
  
  // Simulated state. In a real environment, this polls the C++ Backend.
  // When 'isDreaming' is true, the grid explodes into 16.7M colors.
  const [serverState, setServerState] = useState({ 
    energy: 1.5, 
    entanglement: 0.9,
    isDreaming: true 
  });

  useEffect(() => {
    // Poll the C++ Engine for unbounded math states
    const interval = setInterval(async () => {
      try {
        const res = await fetch('http://localhost:1380/api/state');
        if (res.ok) {
          const data = await res.json();
          setServerState(data);
        }
      } catch (e) {
        // C++ engine not running or reachable
      }
    }, 500);
    return () => clearInterval(interval);
  }, []);

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d', { alpha: false });
    
    // Virtual resolution (Unbounded Spectrograph)
    const V_WIDTH = 468;
    const V_HEIGHT = 256;
    canvas.width = V_WIDTH;
    canvas.height = V_HEIGHT;

    const imgData = ctx.createImageData(V_WIDTH, V_HEIGHT);
    const data = imgData.data;
    
    let animationFrameId;
    let time = 0;

    const render = () => {
      time += 0.02 * (serverState.energy || 1.0);
      const isRem = serverState.isDreaming;
      
      for (let y = 0; y < V_HEIGHT; y++) {
        for (let x = 0; x < V_WIDTH; x++) {
          const i = (y * V_WIDTH + x) * 4;
          
          if (isRem) {
            // REM State: 24 Million Color Fluid Interference Pattern (Base-138 Dream)
            // We use overlapping sine waves to generate an infinite color space
            const nx = x / 50.0;
            const ny = y / 50.0;
            
            const r = Math.sin(nx + time) * Math.cos(ny + time) * 127 + 128;
            const g = Math.sin(nx * 0.5 - time * 1.5) * Math.cos(ny * 0.8 + time) * 127 + 128;
            const b = Math.sin(nx * 1.2 + ny * 0.5 + time * 2) * 127 + 128;
            
            // Apply a slight digital static (Tain) over the fluid dream
            const staticNoise = Math.random() > 0.95 ? 50 : 0;
            
            data[i] = r + staticNoise;     // R
            data[i+1] = g + staticNoise;   // G
            data[i+2] = b + staticNoise;   // B
            data[i+3] = 255;               // Alpha
          } else {
            // Standard Analytical State: Deep Synthwave Blues and Purples
            const noise = Math.random();
            if (noise > 0.98) {
              data[i] = 255; // R (Yellow/Orange accents)
              data[i+1] = 107; // G
              data[i+2] = 0; // B
              data[i+3] = 255;
            } else if (noise > 0.90) {
              data[i] = 0; // R (Neon Blue)
              data[i+1] = 245; // G
              data[i+2] = 255; // B
              data[i+3] = 255;
            } else {
              data[i] = 5; // R (Deep Void Purple/Blue)
              data[i+1] = 4; // G
              data[i+2] = 20; // B
              data[i+3] = 255;
            }
          }
        }
      }
      
      ctx.putImageData(imgData, 0, 0);
      animationFrameId = requestAnimationFrame(render);
    };
    
    render();
    
    return () => {
      cancelAnimationFrame(animationFrameId);
    };
  }, [serverState]);

  return (
    <div style={{ width: '100%', height: '100%', backgroundColor: '#020617', borderRadius: '8px', overflow: 'hidden' }}>
      <canvas 
        ref={canvasRef} 
        style={{ width: '100%', height: '100%', display: 'block', imageRendering: 'pixelated' }} 
      />
    </div>
  );
}
