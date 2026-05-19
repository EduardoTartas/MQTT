# MQTT Broker — Mosquitto no Docker

## Estrutura de pastas

```
mqtt-broker/
├── docker-compose.yml
├── config/
│   └── mosquitto.conf
├── data/          ← persistência (criado automaticamente)
└── log/           ← logs (criado automaticamente)
```

---

## 1. Descobrir o IP da máquina (para configurar no ESP32)

**Linux/Mac:**
```bash
hostname -I | awk '{print $1}'
# ou
ip route get 1 | awk '{print $7}'
```

**Windows (PowerShell):**
```powershell
(Get-NetIPAddress -AddressFamily IPv4 | Where-Object { $_.InterfaceAlias -notmatch "Loopback" }).IPAddress
# ou simplesmente
ipconfig
# → copie o "Endereço IPv4" da interface Wi-Fi/Ethernet
```

---

## 2. Subir o broker

```bash
# Entrar na pasta
cd mqtt-broker

# Criar pastas (caso não existam)
mkdir -p config data log

# Subir o container em background
docker compose up -d

# Ver status
docker compose ps

# Ver logs em tempo real
docker compose logs -f mosquitto
```

---

## 3. Testar o broker

Abra dois terminais:

**Terminal 1 — Assinar o tópico:**
```bash
mosquitto_sub -h localhost -p 1883 -t "iot/grupo01/sensor" -v
```

**Terminal 2 — Publicar mensagem de teste:**
```bash
mosquitto_pub -h localhost -p 1883 -t "iot/grupo01/sensor" \
  -m '{"grupo":"01","valor":42,"envio":0}'
```

---

## 4. Assinar e monitorar dados do ESP32 em tempo real

```bash
mosquitto_sub -h localhost -p 1883 -t "iot/grupo01/sensor" -v
```

Saída esperada:
```
iot/grupo01/sensor {"grupo":"01","valor":73,"envio":1}
iot/grupo01/sensor {"grupo":"01","valor":45,"envio":2}
```

---

## 5. Parar o broker

```bash
docker compose down
```

---

## Saída esperada no monitor serial do ESP32

```
=== IoT MQTT — Grupo 01 ===
Wi-Fi conectado! IP: 192.168.1.42
Conectando ao broker MQTT... Conectado!
[#1] Valor enviado: 73 | Topico: iot/grupo01/sensor
Payload: {"grupo":"01","valor":73,"envio":1}

[#2] Valor enviado: 45 | Topico: iot/grupo01/sensor
Payload: {"grupo":"01","valor":45,"envio":2}
```
