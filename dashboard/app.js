// Droppable Reconnaissance Pod (DRP) — Mission Dashboard Controller

class DRPDashboard {
    constructor() {
        this.simMode = true;
        this.currentScenario = 'clear';
        this.telemetryHistory = [];
        this.animationAngle = 0;

        // Current Telemetry State
        this.state = {
            vbat: 3.92,
            radar: { has_target: false, is_static: false, is_moving: false, dist_cm: 0, static_energy: 0, moving_energy: 0, raw_score: 0 },
            doppler_wake: false,
            imu: { pitch: 0, roll: 0, tilt_deg: 0, w_theta: 1.0, impact_shock: false },
            environment: { co2_ppm: 420, co2_rate: 0.0, temp_c: 26.5, humidity_pct: 58.2 },
            acoustic: { peak_mv: 32, knocks: 0, score: 0 },
            fusion: { confidence_pct: 0, state_code: 0, classification: 'CLEAR / NO TARGET' }
        };

        this.initDOM();
        this.initCanvases();
        this.attachEventListeners();
        this.startLoops();
    }

    initDOM() {
        this.dom = {
            connBadge: document.getElementById('conn-badge'),
            connText: document.getElementById('conn-text'),
            vbatText: document.getElementById('vbat-text'),
            toggleSimBtn: document.getElementById('toggle-sim-btn'),
            exportCsvBtn: document.getElementById('export-csv-btn'),
            clearLogBtn: document.getElementById('clear-log-btn'),
            terminalLog: document.getElementById('terminalLog'),

            // Radar
            radarStateBadge: document.getElementById('radar-state-badge'),
            statDist: document.getElementById('stat-dist'),
            statStaticEnergy: document.getElementById('stat-static-energy'),
            statMovingEnergy: document.getElementById('stat-moving-energy'),

            // Confidence
            classificationBadge: document.getElementById('classification-badge'),
            confidenceCircle: document.getElementById('confidenceCircle'),
            confidenceValue: document.getElementById('confidenceValue'),
            barRadar: document.getElementById('bar-radar'),
            valRadar: document.getElementById('val-radar'),
            barDoppler: document.getElementById('bar-doppler'),
            valDoppler: document.getElementById('val-doppler'),
            barCo2: document.getElementById('bar-co2'),
            valCo2: document.getElementById('val-co2'),
            barAcoustic: document.getElementById('bar-acoustic'),
            valAcoustic: document.getElementById('val-acoustic'),
            valWtheta: document.getElementById('val-wtheta'),

            // IMU
            shockBadge: document.getElementById('shock-badge'),
            imuTilt: document.getElementById('imu-tilt'),
            imuPitch: document.getElementById('imu-pitch'),
            imuRoll: document.getElementById('imu-roll'),
            imuWeight: document.getElementById('imu-weight'),

            // Environmental
            envCo2: document.getElementById('env-co2'),
            envCo2Rate: document.getElementById('env-co2-rate'),
            envTemp: document.getElementById('env-temp'),
            envHumidity: document.getElementById('env-humidity'),
            envAcoustic: document.getElementById('env-acoustic'),
            envKnocks: document.getElementById('env-knocks')
        };
    }

    initCanvases() {
        this.radarCanvas = document.getElementById('radarCanvas');
        this.radarCtx = this.radarCanvas.getContext('2d');

        this.imuCanvas = document.getElementById('imuCanvas');
        this.imuCtx = this.imuCanvas.getContext('2d');
    }

    attachEventListeners() {
        this.dom.toggleSimBtn.addEventListener('click', () => {
            this.simMode = !this.simMode;
            this.dom.toggleSimBtn.textContent = `Sim Mode: ${this.simMode ? 'ON' : 'OFF'}`;
            this.dom.connText.textContent = this.simMode ? 'SIMULATOR ACTIVE' : 'LISTENING (SERIAL/WS)';
            this.log(`Switched to ${this.simMode ? 'Simulation' : 'Hardware Stream'} mode.`, 'info');
        });

        this.dom.exportCsvBtn.addEventListener('click', () => this.exportCsv());
        this.dom.clearLogBtn.addEventListener('click', () => {
            this.dom.terminalLog.innerHTML = '';
        });

        document.querySelectorAll('.scenario-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                document.querySelectorAll('.scenario-btn').forEach(b => b.classList.remove('active'));
                btn.classList.add('active');
                this.setScenario(btn.dataset.scenario);
            });
        });
    }

    setScenario(scenario) {
        this.currentScenario = scenario;
        this.log(`Test Scenario Activated: [${scenario.toUpperCase()}]`, 'info');
    }

    log(msg, type = 'info') {
        const line = document.createElement('div');
        line.className = `term-line ${type}`;
        const time = new Date().toTimeString().split(' ')[0];
        line.textContent = `[${time}] ${msg}`;
        this.dom.terminalLog.appendChild(line);
        this.dom.terminalLog.scrollTop = this.dom.terminalLog.scrollHeight;
    }

    startLoops() {
        // Simulation physics loop (10 Hz)
        setInterval(() => {
            if (this.simMode) {
                this.generateSimData();
            }
            this.updateUI();
            this.recordTelemetry();
        }, 100);

        // Radar & IMU rendering animation loop (60 fps)
        const render = () => {
            this.renderRadarScope();
            this.renderIMUScope();
            requestAnimationFrame(render);
        };
        requestAnimationFrame(render);
    }

    generateSimData() {
        const time = Date.now() / 1000;
        let radarScore = 0, dopplerActive = false, co2Ppm = 420, co2Rate = 0, acousticMv = 30, knocks = 0, pitch = 0, roll = 0;

        switch (this.currentScenario) {
            case 'clear':
                this.state.radar.has_target = false;
                this.state.radar.dist_cm = 0;
                this.state.radar.static_energy = Math.floor(Math.random() * 5);
                this.state.radar.moving_energy = Math.floor(Math.random() * 5);
                break;

            case 'walking':
                this.state.radar.has_target = true;
                this.state.radar.is_moving = true;
                this.state.radar.is_static = false;
                this.state.radar.dist_cm = Math.floor(180 + Math.sin(time) * 100);
                this.state.radar.moving_energy = 85;
                this.state.radar.static_energy = 15;
                dopplerActive = true;
                radarScore = 80;
                break;

            case 'trapped_survivor':
                this.state.radar.has_target = true;
                this.state.radar.is_static = true;
                this.state.radar.is_moving = false;
                this.state.radar.dist_cm = 120;
                this.state.radar.static_energy = 78 + Math.floor(Math.sin(time * 2) * 8); // Breathing micro-motion
                this.state.radar.moving_energy = 10;
                co2Ppm = Math.min(850, 480 + Math.floor((time % 60) * 6));
                co2Rate = 1.25;
                acousticMv = 65;
                radarScore = 75;
                break;

            case 'rubble_noise':
                this.state.radar.has_target = true;
                this.state.radar.dist_cm = 60;
                this.state.radar.static_energy = 35;
                this.state.radar.moving_energy = 30;
                co2Ppm = 422;
                co2Rate = 0.02;
                acousticMv = 240; // Clutter vibration
                radarScore = 40;
                break;

            case 'tilted_pod':
                pitch = 55.0;
                roll = 25.0;
                this.state.radar.has_target = true;
                this.state.radar.dist_cm = 140;
                this.state.radar.static_energy = 60;
                this.state.radar.moving_energy = 10;
                radarScore = 60;
                break;

            case 'knocking':
                this.state.radar.has_target = true;
                this.state.radar.static_energy = 65;
                this.state.radar.dist_cm = 110;
                acousticMv = (Math.floor(time * 3) % 2 === 0) ? 950 : 45;
                knocks = (acousticMv > 800) ? 3 : 0;
                co2Ppm = 540;
                co2Rate = 0.65;
                radarScore = 70;
                break;
        }

        // Compute IMU Tilt & Penalty
        const tiltDeg = Math.sqrt(pitch * pitch + roll * roll);
        const wTheta = Math.max(0.15, Math.cos(tiltDeg * Math.PI / 180));

        this.state.imu = {
            pitch: pitch,
            roll: roll,
            tilt_deg: tiltDeg,
            w_theta: wTheta,
            impact_shock: false
        };

        // Compute Multi-Modal Confidence
        const sRadar = radarScore;
        const sDoppler = dopplerActive ? 100 : 0;
        const sCo2 = Math.min(100, Math.max(0, ((co2Ppm - 420) / 400) * 50 + (co2Rate / 2.0) * 50));
        const sAcoustic = Math.min(100, Math.max(0, ((acousticMv - 50) / 900) * 100));

        const composite = (0.45 * sRadar) + (0.20 * sDoppler) + (0.20 * sCo2) + (0.15 * sAcoustic);
        const finalConf = Math.min(100, Math.max(0, composite * wTheta));

        this.state.radar.raw_score = sRadar;
        this.state.doppler_wake = dopplerActive;
        this.state.environment.co2_ppm = co2Ppm;
        this.state.environment.co2_rate = co2Rate;
        this.state.acoustic.peak_mv = acousticMv;
        this.state.acoustic.knocks = knocks;
        this.state.acoustic.score = sAcoustic;

        let classification = 'CLEAR / NO TARGET';
        let stateCode = 0;
        if (finalConf >= 75) {
            classification = 'CONFIRMED HUMAN PRESENCE';
            stateCode = 3;
        } else if (finalConf >= 50) {
            classification = 'PROBABLE OCCUPANT';
            stateCode = 2;
        } else if (finalConf >= 25) {
            classification = 'UNCERTAIN / AMBIENT NOISE';
            stateCode = 1;
        }

        this.state.fusion = {
            confidence_pct: Math.round(finalConf * 10) / 10,
            state_code: stateCode,
            classification: classification
        };
    }

    updateUI() {
        const s = this.state;

        // Radar stats
        this.dom.radarStateBadge.textContent = s.radar.has_target ? (s.radar.is_static ? 'STATIC TARGET' : 'MOVING TARGET') : 'NO TARGET';
        this.dom.statDist.textContent = s.radar.has_target ? `${s.radar.dist_cm} cm` : '-- cm';
        this.dom.statStaticEnergy.textContent = `${s.radar.static_energy}%`;
        this.dom.statMovingEnergy.textContent = `${s.radar.moving_energy}%`;

        // Confidence
        this.dom.confidenceValue.textContent = `${s.fusion.confidence_pct}%`;
        this.dom.classificationBadge.textContent = s.fusion.classification;
        
        // Dynamic colors for confidence
        let confColor = 'var(--accent-cyan)';
        if (s.fusion.confidence_pct >= 75) confColor = 'var(--accent-red)';
        else if (s.fusion.confidence_pct >= 50) confColor = 'var(--accent-amber)';
        else if (s.fusion.confidence_pct >= 25) confColor = 'var(--accent-blue)';

        this.dom.confidenceCircle.style.borderTopColor = confColor;
        this.dom.confidenceCircle.style.boxShadow = `0 0 25px ${confColor}40`;
        this.dom.classificationBadge.style.color = confColor;
        this.dom.classificationBadge.style.borderColor = confColor;

        // Breakdown bars
        this.dom.barRadar.style.width = `${s.radar.raw_score}%`;
        this.dom.valRadar.textContent = `${Math.round(s.radar.raw_score)}%`;

        this.dom.barDoppler.style.width = s.doppler_wake ? '100%' : '0%';
        this.dom.valDoppler.textContent = s.doppler_wake ? 'ACTIVE' : 'IDLE';

        const co2Score = Math.min(100, Math.max(0, ((s.environment.co2_ppm - 420) / 400) * 50 + (s.environment.co2_rate / 2.0) * 50));
        this.dom.barCo2.style.width = `${co2Score}%`;
        this.dom.valCo2.textContent = `${Math.round(co2Score)}%`;

        this.dom.barAcoustic.style.width = `${s.acoustic.score}%`;
        this.dom.valAcoustic.textContent = `${Math.round(s.acoustic.score)}%`;

        this.dom.valWtheta.textContent = `${s.imu.w_theta.toFixed(2)} (${s.imu.tilt_deg.toFixed(1)}°)`;

        // IMU stats
        this.dom.imuTilt.textContent = `${s.imu.tilt_deg.toFixed(1)}°`;
        this.dom.imuPitch.textContent = `${s.imu.pitch.toFixed(1)}°`;
        this.dom.imuRoll.textContent = `${s.imu.roll.toFixed(1)}°`;
        this.dom.imuWeight.textContent = s.imu.w_theta.toFixed(2);

        // Environmental stats
        this.dom.envCo2.innerHTML = `${s.environment.co2_ppm} <small>ppm</small>`;
        this.dom.envCo2Rate.textContent = `Rate: ${s.environment.co2_rate >= 0 ? '+' : ''}${s.environment.co2_rate.toFixed(2)} ppm/s`;
        this.dom.envAcoustic.innerHTML = `${s.acoustic.peak_mv} <small>mV</small>`;
        this.dom.envKnocks.textContent = `Knocks: ${s.acoustic.knocks}`;
    }

    renderRadarScope() {
        const ctx = this.radarCtx;
        const w = this.radarCanvas.width;
        const h = this.radarCanvas.height;
        const cx = w / 2;
        const cy = h - 20;
        const maxR = h - 40;

        ctx.clearRect(0, 0, w, h);

        // Draw Polar Grid Arcs (1m, 2m, 3m, 4m, 5m)
        ctx.strokeStyle = 'rgba(0, 240, 255, 0.15)';
        ctx.lineWidth = 1;

        for (let i = 1; i <= 5; i++) {
            const r = (maxR / 5) * i;
            ctx.beginPath();
            ctx.arc(cx, cy, r, Math.PI, 2 * Math.PI);
            ctx.stroke();

            // Distance labels
            ctx.fillStyle = 'rgba(148, 163, 184, 0.6)';
            ctx.font = '10px JetBrains Mono';
            ctx.fillText(`${i}m`, cx + r - 18, cy + 12);
        }

        // Draw Radial Angle Lines
        const angles = [Math.PI, Math.PI * 1.15, Math.PI * 1.3, Math.PI * 1.5, Math.PI * 1.7, Math.PI * 1.85, 2 * Math.PI];
        angles.forEach(ang => {
            ctx.beginPath();
            ctx.moveTo(cx, cy);
            ctx.lineTo(cx + Math.cos(ang) * maxR, cy + Math.sin(ang) * maxR);
            ctx.stroke();
        });

        // Rotating radar sweep beam
        this.animationAngle += 0.03;
        const sweepRad = Math.PI + (Math.sin(this.animationAngle) * 0.5 + 0.5) * Math.PI;

        const grad = ctx.createRadialGradient(cx, cy, 10, cx, cy, maxR);
        grad.addColorStop(0, 'rgba(0, 240, 255, 0.4)');
        grad.addColorStop(1, 'rgba(0, 240, 255, 0)');

        ctx.beginPath();
        ctx.moveTo(cx, cy);
        ctx.arc(cx, cy, maxR, sweepRad - 0.15, sweepRad);
        ctx.closePath();
        ctx.fillStyle = grad;
        ctx.fill();

        // Render detected target blip if present
        if (this.state.radar.has_target && this.state.radar.dist_cm > 0) {
            const targetDistRatio = Math.min(1.0, (this.state.radar.dist_cm / 100.0) / 5.0);
            const targetR = targetDistRatio * maxR;
            const targetAngle = Math.PI * 1.5; // Centered on bore-sight

            const tx = cx + Math.cos(targetAngle) * targetR;
            const ty = cy + Math.sin(targetAngle) * targetR;

            // Target Glow Blip
            ctx.beginPath();
            ctx.arc(tx, ty, 8, 0, 2 * Math.PI);
            ctx.fillStyle = this.state.radar.is_static ? '#00f0ff' : '#f59e0b';
            ctx.shadowBlur = 15;
            ctx.shadowColor = ctx.fillStyle;
            ctx.fill();
            ctx.shadowBlur = 0;

            // Blip Target Label
            ctx.fillStyle = '#ffffff';
            ctx.font = 'bold 11px JetBrains Mono';
            ctx.fillText(`${this.state.radar.dist_cm}cm (${this.state.radar.static_energy}%)`, tx + 12, ty - 4);
        }
    }

    renderIMUScope() {
        const ctx = this.imuCtx;
        const w = this.imuCanvas.width;
        const h = this.imuCanvas.height;
        const cx = w / 2;
        const cy = h / 2;

        ctx.clearRect(0, 0, w, h);

        // Draw Artificial Horizon Compass Circle
        ctx.strokeStyle = 'rgba(139, 92, 246, 0.3)';
        ctx.lineWidth = 2;
        ctx.beginPath();
        ctx.arc(cx, cy, 60, 0, 2 * Math.PI);
        ctx.stroke();

        // Pitch & Roll Translation
        const pitchPx = (this.state.imu.pitch / 90.0) * 40;
        const rollRad = (this.state.imu.roll * Math.PI) / 180.0;

        ctx.save();
        ctx.translate(cx, cy);
        ctx.rotate(rollRad);

        // Horizon Line
        ctx.strokeStyle = '#8b5cf6';
        ctx.lineWidth = 3;
        ctx.beginPath();
        ctx.moveTo(-50, pitchPx);
        ctx.lineTo(50, pitchPx);
        ctx.stroke();

        // Pod Orientation Box (Wireframe)
        ctx.strokeStyle = 'rgba(0, 240, 255, 0.8)';
        ctx.lineWidth = 2;
        ctx.strokeRect(-25, pitchPx - 15, 50, 30);

        ctx.restore();

        // Crosshairs
        ctx.strokeStyle = 'rgba(255, 255, 255, 0.15)';
        ctx.lineWidth = 1;
        ctx.beginPath();
        ctx.moveTo(cx - 70, cy); ctx.lineTo(cx + 70, cy);
        ctx.moveTo(cx, cy - 70); ctx.lineTo(cx, cy + 70);
        ctx.stroke();
    }

    recordTelemetry() {
        const row = {
            timestamp_ms: Date.now(),
            scenario: this.currentScenario,
            confidence_pct: this.state.fusion.confidence_pct,
            classification: this.state.fusion.classification,
            radar_score: this.state.radar.raw_score,
            radar_dist_cm: this.state.radar.dist_cm,
            radar_static_energy: this.state.radar.static_energy,
            co2_ppm: this.state.environment.co2_ppm,
            co2_rate: this.state.environment.co2_rate,
            tilt_deg: this.state.imu.tilt_deg,
            w_theta: this.state.imu.w_theta,
            acoustic_mv: this.state.acoustic.peak_mv
        };
        this.telemetryHistory.push(row);
        if (this.telemetryHistory.length > 500) this.telemetryHistory.shift();
    }

    exportCsv() {
        if (this.telemetryHistory.length === 0) {
            alert('No telemetry data to export.');
            return;
        }

        const headers = Object.keys(this.telemetryHistory[0]).join(',');
        const rows = this.telemetryHistory.map(obj => Object.values(obj).join(',')).join('\n');
        const csvContent = 'data:text/csv;charset=utf-8,' + headers + '\n' + rows;

        const encodedUri = encodeURI(csvContent);
        const link = document.createElement('a');
        link.setAttribute('href', encodedUri);
        link.setAttribute('download', `drp_telemetry_exp_${Date.now()}.csv`);
        document.body.appendChild(link);
        link.click();
        document.body.removeChild(link);

        this.log(`Exported ${this.telemetryHistory.length} telemetry records to CSV.`, 'info');
    }
}

// Initialize on page load
window.addEventListener('DOMContentLoaded', () => {
    window.drpDashboard = new DRPDashboard();
});
