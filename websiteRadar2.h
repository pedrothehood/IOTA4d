
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
    
    // NEU: Objekt zum Speichern und Verfolgen der Ziele über Zeit
    let targetHistory = {}; 
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
        });

        requestAnimationFrame(render);
    }

    render();
</script>
</body>
</html>
)rawliteral";