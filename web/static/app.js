const preflightItems = [
  'Responsável operacional definido',
  'Aeronave identificada',
  'Baterias verificadas',
  'Plano/documento de voo anexado quando aplicável',
  'Condições ambientais registradas'
];

const postflightItems = [
  'Logs de voo preservados',
  'Evidências principais anexadas',
  'Ocorrências registradas ou confirmadas ausentes'
];

let lastPosition = null;
let qrStream = null;
let qrDetector = null;
let qrTimer = null;

function renderChecklist(id, items) {
  const root = document.getElementById(id);
  root.innerHTML = '';
  items.forEach((label, index) => {
    const row = document.createElement('label');
    row.className = 'check-row';
    row.innerHTML = `<input type="checkbox" data-group="${id}" data-index="${index}"><span>${label}</span>`;
    root.appendChild(row);
  });
}

function checkedCount(group) {
  return [...document.querySelectorAll(`input[data-group="${group}"]`)].filter(input => input.checked).length;
}

function updateStatus() {
  const pre = checkedCount('preflight');
  const post = checkedCount('postflight');
  const evidence = document.getElementById('evidence').value.trim();
  const occurrences = document.getElementById('occurrences').value.trim();
  const decision = document.getElementById('final-decision').value;
  const status = document.getElementById('mission-status');

  document.getElementById('pre-count').textContent = `${pre}/${preflightItems.length}`;
  document.getElementById('post-count').textContent = `${post}/${postflightItems.length}`;

  let label = 'planejada';
  status.className = 'status-pill';
  if (occurrences || decision === 'bloqueado') {
    label = 'bloqueada';
    status.classList.add('blocked');
  } else if (decision === 'apto_com_restricao') {
    label = 'apta com restrição';
  } else if (pre === preflightItems.length && evidence && post === postflightItems.length) {
    label = 'revisada';
    status.classList.add('reviewed');
  } else if (pre === preflightItems.length && evidence) {
    label = 'executada';
  } else if (pre === preflightItems.length) {
    label = 'apta';
    status.classList.add('ready');
  }
  status.textContent = label;
  document.getElementById('validation-status').textContent = occurrences ? 'bloqueado' : 'rascunho';
  return label;
}

function generateManifest() {
  const status = updateStatus();
  const blocked = status === 'bloqueada';
  const photos = [...document.getElementById('photos').files].map(file => `evidencias/fotos/${file.name}`);
  const manifest = {
    package_id: 'PKG-DRONEOPS-DO-001',
    droneops_version: '0.1.0',
    camposync_contract_version: '0.1.0',
    camponode_id: 'CAMPO-NODE-LOCAL',
    project_id: 'SISTER-CAMPO',
    campaign_id: 'CAMP-2026-001',
    area_id: 'AREA-001',
    mission_id: 'DO-001',
    origin_module: 'droneops',
    human_responsible: document.getElementById('responsible').value,
    aircraft_id: document.getElementById('aircraft').value,
    sensor_id: document.getElementById('sensor').value,
    protocol: {
      schema: 'droneops.protocol_certificate.v1',
      operator_id: document.getElementById('operator-id').value,
      drone_qr: document.getElementById('drone-qr').value,
      latitude: lastPosition?.coords.latitude ?? null,
      longitude: lastPosition?.coords.longitude ?? null,
      accuracy_m: lastPosition?.coords.accuracy ?? null,
      final_decision: document.getElementById('final-decision').value,
      photo_paths: photos
    },
    validation_status: blocked ? 'bloqueado' : 'rascunho',
    sync_status: 'nao_sincronizado',
    expected_destination: 'SisTer',
    mission_status: status,
    files: []
  };
  document.getElementById('manifest-preview').textContent = JSON.stringify(manifest, null, 2);
}

function captureLocation() {
  const target = document.getElementById('gps-status');
  if (!navigator.geolocation) {
    target.textContent = 'GPS indisponível neste navegador';
    return;
  }
  target.textContent = 'Coletando GPS...';
  navigator.geolocation.getCurrentPosition(position => {
    lastPosition = position;
    target.textContent = `${position.coords.latitude.toFixed(6)}, ${position.coords.longitude.toFixed(6)} · ±${Math.round(position.coords.accuracy)} m`;
    updateStatus();
  }, error => {
    target.textContent = `GPS não coletado: ${error.message}`;
  }, {
    enableHighAccuracy: true,
    timeout: 10000,
    maximumAge: 30000
  });
}

async function startQrScan() {
  const video = document.getElementById('qr-video');
  if (!('BarcodeDetector' in window)) {
    alert('Leitura automática de QR não disponível neste navegador. Preencha o campo QR drone manualmente.');
    return;
  }
  qrDetector = new BarcodeDetector({ formats: ['qr_code'] });
  qrStream = await navigator.mediaDevices.getUserMedia({ video: { facingMode: 'environment' } });
  video.srcObject = qrStream;
  video.classList.add('active');
  await video.play();
  qrTimer = window.setInterval(async () => {
    const codes = await qrDetector.detect(video);
    if (codes.length > 0) {
      document.getElementById('drone-qr').value = codes[0].rawValue;
      document.getElementById('aircraft').value = codes[0].rawValue;
      stopQrScan();
      updateStatus();
    }
  }, 600);
}

function stopQrScan() {
  if (qrTimer) {
    window.clearInterval(qrTimer);
    qrTimer = null;
  }
  if (qrStream) {
    qrStream.getTracks().forEach(track => track.stop());
    qrStream = null;
  }
  const video = document.getElementById('qr-video');
  video.pause();
  video.srcObject = null;
  video.classList.remove('active');
}

renderChecklist('preflight', preflightItems);
renderChecklist('postflight', postflightItems);
document.addEventListener('input', updateStatus);
document.addEventListener('change', updateStatus);
updateStatus();
