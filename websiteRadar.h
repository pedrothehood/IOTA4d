
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
    <script>
    const canvas = document.getElementById('radar');
const ctx = canvas.getContext('2d');
const status = document.getElementById('status');
const socket = new WebSocket('ws://' + window.location.hostname + '/ws');
let currentTargets = []; // Speicher für die letzten Daten
socket.onopen = () => status.innerText = "ONLINE";
socket.onclose = () => status.innerText = "OFFLINE";

// Nur Daten empfangen, NICHT zeichnen
socket.onmessage = (event) => {
    try {
        currentTargets = JSON.parse(event.data);
    } catch (e) {
        console.error("JSON Error", e);
    }
};

function drawUI() {
    ctx.strokeStyle = 'rgba(0, 255, 65, 0.15)';
    ctx.lineWidth = 1;
	  ctx.font = "10px monospace";
    // Entfernungskreise
    for(let i=1; i<=4; i++) {
        ctx.beginPath();
        ctx.arc(300, 430, i * 100, Math.PI, 2 * Math.PI);
        ctx.stroke();
        ctx.fillStyle = 'rgba(0, 255, 65, 0.3)';
        ctx.fillText(`${i}m`, 305, 430 - (i * 100));
    }
    // Achsen
    ctx.beginPath();
    ctx.moveTo(300, 430); ctx.lineTo(300, 20);
    ctx.stroke();
}

//neu// Zentrale Render-Schleife (60 FPS)
function render() {
    // Sanfter Motion Blur
    ctx.fillStyle = 'rgba(5, 5, 5, 0.2)'; 
    ctx.fillRect(0, 0, canvas.width, canvas.height);
    
    drawUI();

    currentTargets?.targets?.forEach(t => {
        const x = 300 + (t.x / 10);
        const y = 430 - (t.y / 10);
        const isMoving = Math.abs(t.s) > 5; // Kleiner Schwellenwert für Bewegung
        const color = isMoving ? '255, 50, 50' : '50, 255, 50';

        // Zeichne Ziel
        ctx.fillStyle = `rgb(${color})`;
        ctx.beginPath();
        ctx.arc(x, y, 7, 0, Math.PI * 2);
        ctx.fill();
        
        // Glow-Effekt ohne teures shadowBlur (optional)
        ctx.strokeStyle = `rgba(${color}, 0.3)`;
        ctx.lineWidth = 3;
        ctx.stroke();
        
        ctx.fillStyle = "#fff";
        ctx.fillText(`ID:${t.id} ${t.s}cm/s`, x + 12, y);
    });

	requestAnimationFrame(render);
}
// Starte die Schleife
render();
    </script>
</body>
</html>
)rawliteral";