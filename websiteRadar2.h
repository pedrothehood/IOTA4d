
// HTML mit Canvas für die visuelle Darstellung
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<style>
body {
    background: #050505;
    color: #00ff41;
    font-family: 'Courier New', monospace;
    display: flex;
    flex-direction: column;
    align-items: center;
    margin: 0;
    overflow: hidden;
}
h1 { font-size: 1.2rem; letter-spacing: 4px; margin: 20px 0; }
canvas {
    background: #000;
    border: 1px solid #00ff41;
    box-shadow: 0 0 15px rgba(0, 255, 65, 0.2);
    border-radius: 300px 300px 0 0;
}
#status { margin-top: 10px; font-size: 0.8rem; }
</style>
    <meta charset="UTF-8">
    <title>RD-03D Radar Monitor</title>
</head>
<body>
    <h1>RD-03D REALTIME RADAR</h1>
    <canvas id="radar" width="600" height="450"></canvas>
    <div id="status">Verbinde WebSocket...</div>
	<div id="btnDiv">
	 <button class="btn" id="startBtn">1. AUDIO INITIALISIEREN</button>
	 <button class="btn" id="muteBtn" disabled>SOUND AUSSCHALTEN</button>
	
	</div>
	
	
<script>
    const canvas = document.getElementById('radar');
    const ctx = canvas.getContext('2d');
    const status = document.getElementById('status');
    const socket = new WebSocket('ws://' + window.location.hostname + '/ws');
     // NEU: Objekt zum Speichern und Verfolgen der Ziele über Zeit
    let targetHistory = {};   // nach vorne genommen wegen Sound

	// Globale Steuerungs-Variablen für das Audio-System
	let trackerAudioCtx = null;
	let isTrackerMuted = false;
	let currentTrackerDistanceCm = 800;
	let trackerClickTimeoutId = null;
     const startBtn = document.getElementById('startBtn');
        const muteBtn = document.getElementById('muteBtn');
           let playbackIntervalId = null;

           function setInitSound() {
             if (!isTrackerMuted) {
                // kleinstes y ermitteln:
               if (targetHistory && Object.keys(targetHistory).length > 0){
                     // 1. Alle y-Werte in ein Array extrahieren
                    const yWerte = Object.values(targetHistory).map(obj => obj.y);

                    // 2. Das kleinste Element finden
                     let kleinstesY = Math.min(...yWerte);
                    kleinstesY = kleinstesY / 10;
                    // Füttert die API direkt mit dem neuen Wert für Echtzeit-Klicks-Anpassung
                   playAlienTrackerSound(kleinstesY); 
            
                  // Trigger-Rhythmus sofort anpassen
                 startAudioTriggerLoop();
                  }

   //playAlienTrackerSound(parseInt(slider.value));  // ab muteButton
  
              }
    //console.log("Wird jede Sekunde ausgeführt");
          }

        // 1000 Millisekunden = 1 Sekunde
            setInterval(setInitSound, 3000);
		
		 startBtn.addEventListener('click', () => {
            if (!trackerAudioCtx) {
                trackerAudioCtx = new (window.AudioContext || window.webkitAudioContext)();
                playTrackerClickLoop(); // Startet Hintergrundklicken
                
                // UI freischalten
               // slider.disabled = false;
                muteBtn.disabled = false;
                startBtn.style.display = 'none';

                // Automatische Intervall-Schleife simuliert das regelmäßige Auslösen des Pings
                startAudioTriggerLoop();
            }
        });

        muteBtn.addEventListener('click', () => {
            toggleTrackerAudio();
            if (isTrackerMuted) {
                muteBtn.innerText = "SOUND EINSCHALTEN";
                muteBtn.style.backgroundColor = "#dc3545";
            } else {
                muteBtn.innerText = "SOUND AUSSCHALTEN";
                muteBtn.style.backgroundColor = "#6c757d";
              //  playAlienTrackerSound(parseInt(slider.value));
              playAlienTrackerSound(790);
            }
        });
		
	/**
 * Schaltet das gesamte Tracking-Audiosystem (Pings und Klicks) an oder aus.
 * @param {boolean|null} forceState - Optional: true für Stumm, false für Ton an.
 */
function toggleTrackerAudio(forceState = null) {
    isTrackerMuted = (forceState !== null) ? forceState : !isTrackerMuted;

    if (isTrackerMuted) {
        if (trackerClickTimeoutId) {
            clearTimeout(trackerClickTimeoutId);
            trackerClickTimeoutId = null;
        }
    } else {
        playTrackerClickLoop();
    }
}

/**
 * Kernfunktion: Verarbeitet die Distanz und löst den passenden Alarm-Ping aus.
 * @param {number} distanceCm - Die aktuelle Distanz in Zentimetern (0 bis 800).
 */
function playAlienTrackerSound(distanceCm) {
    if (Object.keys(targetHistory).length === 0) return;
    currentTrackerDistanceCm = distanceCm;

    // Abbruchbedingungen: Stummgeschaltet, über 790cm oder AudioContext nicht bereit
    if (isTrackerMuted || distanceCm > 790 || !trackerAudioCtx || trackerAudioCtx.state === 'suspended') {
        return;
    }

    let safeDistance = Math.min(Math.max(distanceCm, 0), 800);
    let intensity = 1 - (safeDistance / 800);

    let osc1 = trackerAudioCtx.createOscillator();
    let osc2 = trackerAudioCtx.createOscillator();
    let gainNode = trackerAudioCtx.createGain();

    osc1.type = 'sawtooth';
    osc2.type = 'sine';

    let startFreq1 = 220 + (intensity * 730); 
    let startFreq2 = 224 + (intensity * 730); 

    let endFreq1 = startFreq1 * 0.45;
    let endFreq2 = startFreq2 * 0.45;

    osc1.frequency.setValueAtTime(startFreq1, trackerAudioCtx.currentTime);
    osc1.frequency.exponentialRampToValueAtTime(endFreq1, trackerAudioCtx.currentTime + 0.18);

    osc2.frequency.setValueAtTime(startFreq2, trackerAudioCtx.currentTime);
    osc2.frequency.exponentialRampToValueAtTime(endFreq2, trackerAudioCtx.currentTime + 0.18);

    gainNode.gain.setValueAtTime(0.6, trackerAudioCtx.currentTime);
    gainNode.gain.exponentialRampToValueAtTime(0.001, trackerAudioCtx.currentTime + 0.35);

    osc1.connect(gainNode);
    osc2.connect(gainNode);
    gainNode.connect(trackerAudioCtx.destination);
    
    osc1.start();
    osc2.start();
    osc1.stop(trackerAudioCtx.currentTime + 0.36);
    osc2.stop(trackerAudioCtx.currentTime + 0.36);
}

 function playTrackerClickLoop() {
    if (Object.keys(targetHistory).length === 0) return;
            if (isTrackerMuted || !trackerAudioCtx || trackerAudioCtx.state === 'suspended') return;
            if (currentTrackerDistanceCm <= 790) {
                let intensity = 1 - (Math.min(Math.max(currentTrackerDistanceCm, 0), 800) / 800);
                let osc = trackerAudioCtx.createOscillator();
                let gainNode = trackerAudioCtx.createGain();
                let filterNode = trackerAudioCtx.createBiquadFilter();
                osc.type = 'triangle'; osc.frequency.setValueAtTime(65, trackerAudioCtx.currentTime);
                filterNode.type = 'lowpass'; filterNode.frequency.setValueAtTime(200, trackerAudioCtx.currentTime);
                gainNode.gain.setValueAtTime(0.8, trackerAudioCtx.currentTime);
                gainNode.gain.exponentialRampToValueAtTime(0.001, trackerAudioCtx.currentTime + 0.02);
                osc.connect(filterNode); filterNode.connect(gainNode); gainNode.connect(trackerAudioCtx.destination);
                osc.start(); osc.stop(trackerAudioCtx.currentTime + 0.025);
                let nextClick = 900 - (intensity * 835);
                trackerClickTimeoutId = setTimeout(playTrackerClickLoop, Math.max(nextClick, 65));
            } else {
                trackerClickTimeoutId = setTimeout(playTrackerClickLoop, 200);
            }
        }
   // Berechnet die Zeitintervalle für den Ping-Aufruf (identisch zur Tracker-Logik)
        function startAudioTriggerLoop() {
            if (playbackIntervalId) clearInterval(playbackIntervalId);
            // pedro
            //const t = targetHistory[id];
            let val = 790;
           
           

                if (targetHistory && Object.keys(targetHistory).length > 0){
                     // 1. Alle y-Werte in ein Array extrahieren
                    const yWerte = Object.values(targetHistory).map(obj => obj.y);

                    // 2. Das kleinste Element finden
                     let kleinstesY = Math.min(...yWerte);
                    val = kleinstesY / 10;
                   
                }





            let ratio = val / 800;
            let currentPingInterval = 200 + (ratio * ratio) * (5000 - 200);
              
            playbackIntervalId = setInterval(() => {
              //  playAlienTrackerSound(parseInt(slider.value));
               playAlienTrackerSound(val);
            }, currentPingInterval);
        }


    // NEU: Objekt zum Speichern und Verfolgen der Ziele über Zeit
   // let targetHistory = {}; 
    const MAX_TIMEOUT_MS = 500; // Maximale Haltezeit (0.5 Sekunden)

    socket.onopen = () => status.innerText = 'ONLINE';
    socket.onclose = () => status.innerText = 'OFFLINE';

    socket.onmessage = (event) => {
        try {
            const data = JSON.parse(event.data);
            const targets = data.targets ? data.targets : data;
            const now = Date.now();

            if (Array.isArray(targets)) {
                targets.forEach(t => {
                    if (t.x === 0 && t.y === 0) return; // Rauschen filtern

                    // Ziel in History speichern oder aktualisieren
                    targetHistory[t.id] = {
                        x: t.x,
                        y: t.y,
                        s: t.s,
                        lastSeen: now
                    };
                    //const small_y = t.y / 10;
                    //playAlienTrackerSound(small_y);
                });
            }
        } catch (e) {
            console.error('JSON Error', e);
        }
    };

    function drawUI() {
        ctx.strokeStyle = 'rgba(0, 255, 65, 0.15)';
        ctx.lineWidth = 1;
        ctx.font = '20px monospace';
        
        const scale = 15;
        const originX = 300;
        const originY = 430;

        for(let i = 1; i <= 6; i++) {
            const radius = (i * 1000) / scale;
            if (originY - radius < 10) break;

            ctx.beginPath();
            ctx.arc(originX, originY, radius, Math.PI, 2 * Math.PI);
            ctx.stroke();
            
            ctx.fillStyle = 'rgba(0, 255, 65, 0.3)';
            ctx.fillText(`${i}m`, originX + 5, originY - radius - 2);
        }

        ctx.beginPath();
        ctx.moveTo(originX, originY);
        ctx.lineTo(originX, 20);
        ctx.stroke();

        // ±60° Begrenzungslinien
        ctx.save();
        ctx.strokeStyle = 'rgba(255, 65, 65, 0.3)';
        ctx.lineWidth = 1.5;
        
        const maxRadius = 410;
        
        ctx.beginPath();
        ctx.moveTo(originX, originY);
        ctx.lineTo(originX + Math.cos(Math.PI * 1.166) * maxRadius, originY + Math.sin(Math.PI * 1.166) * maxRadius);
        ctx.stroke();

        ctx.beginPath();
        ctx.moveTo(originX, originY);
        ctx.lineTo(originX + Math.cos(Math.PI * 1.833) * maxRadius, originY + Math.sin(Math.PI * 1.833) * maxRadius);
        ctx.stroke();
        
        ctx.restore();
    }

    function render() {
        // Sanfter Motion Blur
        ctx.fillStyle = 'rgba(5, 5, 5, 0.2)';
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        drawUI();

        const now = Date.now();

        // Iteriere über alle gespeicherten Ziele
        Object.keys(targetHistory).forEach(id => {
            const t = targetHistory[id];
            const age = now - t.lastSeen;

            // NEU: Zu alte Ziele sofort aus dem Speicher löschen
            if (age > MAX_TIMEOUT_MS) {
                delete targetHistory[id];
                return;
            }

            // Berechne Schwindungs-Faktor (1.0 = frisch, gegen 0.0 = verschwindend)
            const fadeAlpha = 1.0 - (age / MAX_TIMEOUT_MS);

            // Winkel berechnen
            const angleRad = Math.atan2(Math.abs(t.x), t.y);
            const angleDeg = angleRad * (180 / Math.PI);
            const isOutside = angleDeg > 60;

            const scale = 15;
            const x = 300 + (t.x / 8); //(t.x / scale);
            const y = 430 - (t.y / scale);

            // Grundfarben und Texttransparenz definieren
            let baseColor;
            let textAlpha;

            if (isOutside) {
                baseColor = `rgba(100, 100, 100, ${0.4 * fadeAlpha})`;
                textAlpha = `rgba(255, 255, 255, ${0.3 * fadeAlpha})`;
            } else {
                const isMoving = Math.abs(t.s) > 5;
                const rgb = isMoving ? '255, 50, 50' : '50, 255, 50';
                baseColor = `rgba(${rgb}, ${fadeAlpha})`;
                textAlpha = `rgba(255, 255, 255, ${0.85 * fadeAlpha})`;
            }

            // Zeichne Ziel
            ctx.fillStyle = baseColor;
            ctx.beginPath();
            ctx.arc(x, y, 7, 0, Math.PI * 2);
            ctx.fill();

            // Glow-Effekt (verblasst ebenfalls)
            if (!isOutside) {
                ctx.strokeStyle = baseColor.replace(`, ${fadeAlpha})`, `, ${0.3 * fadeAlpha})`);
                ctx.lineWidth = 3;
                ctx.stroke();
            }

            // Text-Informationen
            ctx.fillStyle = textAlpha;
            ctx.fillText(`ID:${id} ${t.s}cm/s  x= ${t.x} y= ${t.y}`, x + 12, y);
            //playAlienTrackerSound(t.y);
        });

        requestAnimationFrame(render);
    }

    render();
</script>
</body>
</html>
)rawliteral";