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

// --- Toasts ---
function showToast(message, type = 'info') {
  const container = document.getElementById('toast-container');
  if (!container) return;
  const toast = document.createElement('div');
  toast.className = `toast ${type}`;
  toast.textContent = message;
  container.appendChild(toast);
  setTimeout(() => {
    toast.classList.add('fade-out');
    toast.addEventListener('animationend', () => toast.remove());
  }, 3000);
}

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

  // Atualizar cabeçalho
  const missionId = document.getElementById('mission').value || 'Nova Missão';
  document.getElementById('display-mission-id').textContent = missionId;
  const proj = document.getElementById('project').value || 'Projeto';
  const camp = document.getElementById('campaign').value || 'Campanha';
  const area = document.getElementById('area').value || 'Área';
  document.getElementById('display-mission-context').textContent = `${proj} · ${camp} · ${area}`;

  return label;
}

// --- LocalStorage persistence ---
function saveDraft() {
  const missionId = document.getElementById('mission').value || 'draft';
  const payload = {
    project: document.getElementById('project').value,
    campaign: document.getElementById('campaign').value,
    area: document.getElementById('area').value,
    mission: document.getElementById('mission').value,
    responsible: document.getElementById('responsible').value,
    operatorId: document.getElementById('operator-id').value,
    aircraft: document.getElementById('aircraft').value,
    droneQr: document.getElementById('drone-qr').value,
    sensor: document.getElementById('sensor').value,
    date: document.getElementById('date').value,
    evidence: document.getElementById('evidence').value,
    occurrences: document.getElementById('occurrences').value,
    finalDecision: document.getElementById('final-decision').value,
    preflight: [...document.querySelectorAll('input[data-group="preflight"]')].map(i => i.checked),
    postflight: [...document.querySelectorAll('input[data-group="postflight"]')].map(i => i.checked)
  };
  localStorage.setItem(`droneops_draft_${missionId}`, JSON.stringify(payload));
  // Save last edited mission id
  localStorage.setItem('droneops_last_mission', missionId);

  // Sincroniza com a API nativa do CampoNode se estivermos online
  if (navigator.onLine) {
    syncApi(payload);
  }
}

async function syncApi(payload) {
  try {
    const res = await fetch('/api/missions', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        mission_id: payload.mission,
        project_id: payload.project,
        campaign_id: payload.campaign,
        area_id: payload.area,
        responsible: payload.responsible,
        operator_id: payload.operatorId,
        aircraft_id: payload.aircraft,
        drone_qr: payload.droneQr,
        sensor_id: payload.sensor,
        planned_date: payload.date,
        evidence_paths: payload.evidence,
        occurrences: payload.occurrences,
        final_decision: payload.finalDecision,
        preflight: payload.preflight,
        postflight: payload.postflight
      })
    });
    if (!res.ok) throw new Error('API Sync Failed');
  } catch (e) {
    console.warn('Falha ao sincronizar com backend C++', e);
  }
}

async function loadDraft() {
  const missionId = localStorage.getItem('droneops_last_mission') || 'DO-001';
  
  // Tenta carregar do backend nativo primeiro
  try {
    const res = await fetch(`/api/missions/${missionId}`);
    if (res.ok) {
      const data = await res.json();
      document.getElementById('project').value = data.project_id || '';
      document.getElementById('campaign').value = data.campaign_id || '';
      document.getElementById('area').value = data.area_id || '';
      document.getElementById('mission').value = data.mission_id || '';
      document.getElementById('responsible').value = data.responsible || '';
      document.getElementById('operator-id').value = data.operator_id || '';
      document.getElementById('aircraft').value = data.aircraft_id || '';
      document.getElementById('drone-qr').value = data.drone_qr || '';
      document.getElementById('sensor').value = data.sensor_id || '';
      document.getElementById('date').value = data.planned_date || '';
      document.getElementById('evidence').value = data.evidence_paths || '';
      document.getElementById('occurrences').value = data.occurrences || '';
      document.getElementById('final-decision').value = data.final_decision || 'rascunho';
      document.getElementById('sync-status').textContent = (data.sync_status || 'nao_sincronizado').replace('_', ' ');

      const pres = document.querySelectorAll('input[data-group="preflight"]');
      (data.preflight || []).forEach((checked, i) => { if (pres[i]) pres[i].checked = checked; });

      const posts = document.querySelectorAll('input[data-group="postflight"]');
      (data.postflight || []).forEach((checked, i) => { if (posts[i]) posts[i].checked = checked; });

      updateStatus();
      showToast(`Missão ${missionId} carregada do servidor local.`, 'success');
      
      // Ajuste automático da versão do sistema
      fetch('/api/version').then(v => v.json()).then(vd => {
        if(vd.version) document.getElementById('app-version').textContent = vd.version;
      }).catch(e => console.warn('Erro ao ler versão', e));
      
      return;
    }
  } catch (e) {
    console.warn('Backend inativo, tentando localStorage...');
  }

  // Fallback: localStorage
  const saved = localStorage.getItem(`droneops_draft_${missionId}`);
  if (saved) {
    try {
      const data = JSON.parse(saved);
      document.getElementById('project').value = data.project || '';
      document.getElementById('campaign').value = data.campaign || '';
      document.getElementById('area').value = data.area || '';
      document.getElementById('mission').value = data.mission || '';
      document.getElementById('responsible').value = data.responsible || '';
      document.getElementById('operator-id').value = data.operatorId || '';
      document.getElementById('aircraft').value = data.aircraft || '';
      document.getElementById('drone-qr').value = data.droneQr || '';
      document.getElementById('sensor').value = data.sensor || '';
      document.getElementById('date').value = data.date || '';
      document.getElementById('evidence').value = data.evidence || '';
      document.getElementById('occurrences').value = data.occurrences || '';
      document.getElementById('final-decision').value = data.finalDecision || 'rascunho';

      const pres = document.querySelectorAll('input[data-group="preflight"]');
      (data.preflight || []).forEach((checked, i) => { if (pres[i]) pres[i].checked = checked; });

      const posts = document.querySelectorAll('input[data-group="postflight"]');
      (data.postflight || []).forEach((checked, i) => { if (posts[i]) posts[i].checked = checked; });

      showToast(`Rascunho da missão ${missionId} carregado.`, 'success');
    } catch (e) {
      console.error('Erro ao carregar draft', e);
    }
  }
}

async function generateManifest() {
  const status = updateStatus();
  const blocked = status === 'bloqueada';
  saveDraft(); // Garante salvamento no SQLite

  const missionId = document.getElementById('mission').value;
  if(!missionId) return showToast('Missão inválida', 'error');

  showToast('Empacotando missão localmente...', 'info');
  try {
    const res = await fetch(`/api/missions/${missionId}/package`, { method: 'POST' });
    if(res.ok) {
      document.getElementById('sync-status').textContent = 'pendente sincronizacao';
      showToast('Pacote CampoSync gerado e pendente de sincronização.', 'success');
      document.getElementById('manifest-preview').textContent = "Pacote salvo localmente no dispositivo.";
    } else {
      throw new Error('Falha no empacotamento');
    }
  } catch(e) {
    showToast('Erro ao gerar pacote.', 'error');
  }
}

async function syncCloud() {
  showToast('Sincronizando com SISTER-Observa...', 'info');
  try {
    const res = await fetch('/api/system/sync', { method: 'POST' });
    if(res.ok) {
      const data = await res.json();
      showToast(`${data.synced} missão(ões) enviada(s) para a nuvem!`, 'success');
      loadDraft(); // Recarrega status
    } else {
      throw new Error('Erro na sincronização');
    }
  } catch(e) {
    showToast('Falha ao enviar pacotes.', 'error');
  }
}

async function checkNetwork() {
  try {
    const res = await fetch('/api/system/network');
    const data = await res.json();
    const netStatus = document.getElementById('network-status');
    const btnSync = document.getElementById('btn-sync');
    if(data.online) {
      netStatus.textContent = 'Online';
      netStatus.style.color = '#22c55e';
      btnSync.disabled = false;
    } else {
      netStatus.textContent = 'Offline';
      netStatus.style.color = '#ff4444';
      btnSync.disabled = true;
    }
  } catch(e) {
    // Falha na API
  }
}

// Pooling rede a cada 10s
setInterval(checkNetwork, 10000);
checkNetwork();

function captureLocation() {
  const target = document.getElementById('gps-status');
  
  if (!window.isSecureContext) {
    target.textContent = 'Bloqueado (HTTPS exigido)';
    showToast('No iOS ou Android modernos, o GPS exige conexão HTTPS (segura) ou localhost.', 'error');
    return;
  }

  if (!navigator.geolocation) {
    target.textContent = 'GPS indisponível neste navegador';
    return;
  }
  target.textContent = 'Coletando GPS...';
  navigator.geolocation.getCurrentPosition(position => {
    lastPosition = position;
    target.textContent = `${position.coords.latitude.toFixed(6)}, ${position.coords.longitude.toFixed(6)} · ±${Math.round(position.coords.accuracy)} m`;
    showToast('Coordenadas GPS capturadas.', 'success');
    updateStatus();
    saveDraft();
  }, error => {
    target.textContent = `GPS não coletado: ${error.message}`;
    showToast(`Erro no GPS: ${error.message}`, 'error');
  }, {
    enableHighAccuracy: true,
    timeout: 10000,
    maximumAge: 30000
  });
}

async function startQrScan() {
  const video = document.getElementById('qr-video');
  
  if (!window.isSecureContext) {
    alert('Bloqueado pela segurança do iOS/Navegador: O acesso à câmera exige HTTPS (conexão segura) ou acesso via localhost (127.0.0.1).');
    return;
  }

  if (!('BarcodeDetector' in window)) {
    alert('Leitura automática de QR não disponível neste navegador (ou requer ativação). Preencha o campo QR drone manualmente.');
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
      showToast('QR Code capturado com sucesso!', 'success');
      stopQrScan();
      updateStatus();
      saveDraft();
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

loadDraft();

document.addEventListener('input', () => {
  updateStatus();
  saveDraft();
});
document.addEventListener('change', () => {
  updateStatus();
  saveDraft();
});

updateStatus();
