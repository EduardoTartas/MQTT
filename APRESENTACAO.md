# Trabalho IoT — Publicação de Dados Aleatórios via MQTT
## Grupo 01 | Disciplina: Internet das Coisas

---

## 1. Objetivo

Desenvolver um dispositivo IoT com ESP32 capaz de:

1. Conectar-se ao Wi-Fi
2. Gerar valores aleatórios entre 0 e 100
3. Conectar-se a um broker MQTT
4. Publicar os valores periodicamente (a cada 5 segundos)
5. Visualizar os dados em um cliente MQTT

---

## 2. Arquitetura do Sistema

```mermaid
graph TD
    %% Origem dos dados (Dispositivo IoT)
    subgraph ESP32 ["ESP32 (Nó Sensor IoT)"]
        ADC["ADC (Pino A0 flutuante)"] -->|Ruído Térmico| Seed["randomSeed()"]
        Seed -->|Semente Aleatória| Random["random(0, 101)"]
        Random -->|Gera Valor| JSON["Payload JSON\n{'grupo':'01', 'valor':X, 'envio':N}"]
        WiFi["WiFi.begin()"] -->|Conexão TCP/IP| ClientTCP["WiFiClient Socket"]
        JSON --> ClientTCP
    end

    %% Servidor de Mensagens (Broker)
    subgraph Servidor ["Broker MQTT (Docker)"]
        Mosquitto["Eclipse Mosquitto 2.0"]
        ClientTCP -->|Porta 1883 (TCP)| Mosquitto
    end

    %% Clientes Finais (Assinantes)
    subgraph Consumidores ["Clientes Assinantes (Subscribers)"]
        Terminal["Console Terminal\n(mosquitto_sub)"]
        Browser["Dashboard Web\n(Browser)"]
        
        Mosquitto -->|Filtro: iot/grupo01/sensor| Terminal
        Mosquitto -->|Porta 9001 (WebSockets)| Browser
    end

    %% Detalhes do Front-end
    subgraph Front ["Processamento Dashboard (HTML/JS)"]
        Browser -->|mqtt.js| Parse["JSON.parse()"]
        Parse -->|Chart.js| Chart["Gráfico em Tempo Real"]
        Parse -->|Manipulação DOM| Cards["Cards de Status & Log Histórico"]
    end
    
    %% Estilização do Diagrama
    classDef esp fill:#1e293b,stroke:#3b82f6,stroke-width:2px,color:#f8fafc;
    classDef broker fill:#1e293b,stroke:#22c55e,stroke-width:2px,color:#f8fafc;
    classDef sub fill:#1e293b,stroke:#eab308,stroke-width:2px,color:#f8fafc;
    classDef dash fill:#1e293b,stroke:#ec4899,stroke-width:2px,color:#f8fafc;
    
    class ESP32,ADC,Seed,Random,JSON,WiFi,ClientTCP esp;
    class Servidor,Mosquitto broker;
    class Consumidores,Terminal,Browser sub;
    class Front,Parse,Chart,Cards dash;
```

**Esquema de Rede (ASCII):**
```
┌─────────────┐        Wi-Fi / TCP        ┌──────────────────────┐
│   ESP32     │ ────── MQTT:1883 ────────► │  Broker Mosquitto    │
│  (publicador)│                           │  (Docker Container)  │
└─────────────┘                           └──────────┬───────────┘
                                                     │
                    ┌────────────────────────────────┤
                    │                                │
          ┌─────────▼──────────┐        ┌───────────▼──────────┐
          │  mosquitto_sub     │        │  Dashboard HTML       │
          │  (terminal)        │        │  (browser)            │
          │  assinante MQTT    │        │  WebSocket:9001       │
          └────────────────────┘        └──────────────────────┘
```

**Fluxo de dados:**
1. ESP32 gera número aleatório e monta payload JSON
2. ESP32 publica no broker via TCP (porta 1883)
3. Broker repassa para todos os assinantes do tópico `iot/grupo01/sensor`
4. Dashboard recebe via WebSocket e atualiza gráfico em tempo real

---

## 3. Tecnologias Utilizadas

| Componente | Tecnologia | Justificativa |
|---|---|---|
| Hardware | ESP32 DOIT DevKit V1 | Wi-Fi nativo, amplamente suportado |
| Firmware | Arduino (PlatformIO) | Bibliotecas maduras, serial monitor integrado |
| Biblioteca MQTT | PubSubClient v2.8 | Leve, compatível com ESP32, amplamente documentada |
| Broker MQTT | Eclipse Mosquitto 2.0 | Open-source, confiável, padrão da indústria |
| Infraestrutura | Docker + Docker Compose | Portável, sobe em qualquer máquina com um comando |
| Dashboard | HTML + Chart.js + mqtt.js | Single-file, sem instalação, roda direto no browser |

---

## 4. Estrutura do Projeto

```
MQTT/
├── src/
│   └── main.cpp              ← firmware do ESP32
├── mqtt-broker/
│   ├── docker-compose.yml    ← define o container Mosquitto
│   ├── config/
│   │   └── mosquitto.conf    ← configuração do broker
│   ├── data/                 ← persistência de mensagens retidas
│   └── log/                  ← logs do broker
├── dashboard/
│   └── index.html            ← dashboard web em tempo real
├── platformio.ini            ← configuração do projeto (framework + libs)
└── Makefile                  ← automação de todos os comandos
```

---

## 5. Requisitos Implementados

### Obrigatórios ✅

| Requisito | Implementação |
|---|---|
| Conectar ao Wi-Fi | `WiFi.begin()` em `conectarWiFi()` com feedback serial |
| Conectar ao broker MQTT | `mqttClient.connect()` em `conectarMQTT()` |
| Gerar números aleatórios | `random(0, 101)` com semente de `analogRead(0)` |
| Publicar em tópico MQTT | `mqttClient.publish(MQTT_TOPICO, payload)` |
| Enviar a cada 5 segundos | `millis()` sem `delay()` |
| Monitor serial (115200 baud) | `Serial.printf()` em cada envio |

### Adicionais (Diferenciais) ✅

| Requisito | Implementação | Por que escolhemos |
|---|---|---|
| Payload JSON | `{"grupo":"01","valor":73,"envio":1}` | Formato padrão para IoT, fácil de parsear |
| Reconexão automática Wi-Fi | Verificação em `loop()` com `WiFi.isConnected()` | Robustez em ambientes instáveis |
| Reconexão automática MQTT | Verificação em `loop()` com `mqttClient.connected()` | Evita paradas silenciosas |
| Contador de envios | Campo `envio` incrementado a cada publicação | Rastreia mensagens perdidas |
| Dashboard web em tempo real | HTML com Chart.js + mqtt.js via WebSocket | Visual para a apresentação |

---

## 6. Estrutura do Tópico MQTT

```
iot/grupo01/sensor
 │    │       └── tipo de dado (sensor genérico)
 │    └────────── identificação do grupo
 └─────────────── área/domínio (internet das coisas)
```

O protocolo MQTT usa **tópicos hierárquicos** separados por `/`.
Assinantes podem usar wildcards: `iot/+/sensor` (um nível) ou `iot/#` (qualquer nível).

---

## 7. Payload JSON

A cada publicação, o ESP32 envia:

```json
{"grupo":"01","valor":73,"envio":5}
```

| Campo | Tipo | Descrição |
|---|---|---|
| `grupo` | string | Identificação do grupo (fixo: "01") |
| `valor` | inteiro | Número aleatório gerado (0–100) |
| `envio` | inteiro | Contador de publicações desde o boot |

---

## 8. Saída no Monitor Serial

```
=== IoT MQTT — Grupo 01 ===
Conectando ao Wi-Fi: VHA-LAB1L-007....
Wi-Fi conectado! IP: 10.42.0.123
Conectando ao broker MQTT... Conectado!

[#1] Valor enviado: 73 | Topico: iot/grupo01/sensor
Payload: {"grupo":"01","valor":73,"envio":1}

[#2] Valor enviado: 45 | Topico: iot/grupo01/sensor
Payload: {"grupo":"01","valor":45,"envio":2}
```

---

## 9. Como Reproduzir o Projeto

### Pré-requisitos

```bash
# Instalar clientes Mosquitto (para testar no terminal)
sudo apt-get install -y mosquitto-clients

# Docker e Docker Compose devem estar instalados
docker --version && docker compose version
```

### Passo a passo

**1. Descobrir o IP do computador na rede**
```bash
hostname -I | awk '{print $1}'
# Exemplo de saída: 10.42.0.164
```

**2. Configurar `src/main.cpp`**
```cpp
const char* WIFI_SSID   = "nome-da-rede";   // rede 2.4 GHz
const char* WIFI_SENHA  = "senha";
const char* MQTT_BROKER = "10.42.0.164";    // IP do computador
```

**3. Gravar o ESP32**
```bash
make flash
# Segurar botão BOOT no ESP32 se aparecer "Connecting..."
```

**4. Subir o broker**
```bash
make broker
```

**5. Monitorar (serial + MQTT)**
```bash
make watch
# Abre 2 abas: monitor serial e mosquitto_sub
```

**6. Abrir o dashboard**
```bash
make dashboard
# Abre o browser em dashboard/index.html
# Inserir IP do broker e clicar "Conectar"
```

**7. (Opcional) Testar sem ESP32**
```bash
make test-pub
# Publica uma mensagem de teste no broker
```

---

## 10. Decisões Técnicas e Paradigmas

### Paradigma Bare Metal Super Loop com Multitarefa Cooperativa

Embora o ESP32 seja uma plataforma moderna que roda o sistema operacional de tempo real **FreeRTOS** sob a camada do Arduino Core, optamos por implementar o firmware utilizando o paradigma clássico de **Bare Metal Super Loop** com **Multitarefa Cooperativa**. Isso foi feito por meio de:

1. **Execução Sequencial Limpa:** O código roda inteiramente no Core 1 do chip através da função `loop()`, minimizando a complexidade associada à troca de contexto e concorrência preemptiva.
2. **Multitarefa Não-Bloqueante:** Em vez de depender do agendador de tarefas do RTOS para alternar threads (o que aumentaria a pegada de memória), o firmware utiliza o método de *polling* temporal cooperativo.

### Por que `millis()` em vez de `delay()`?

O `delay(5000)` é uma chamada síncrona/bloqueante que **congela o processador** por 5 segundos. Em um sistema bare-metal típico, durante esse congelamento:
* A função `mqttClient.loop()` (que atua como uma bomba de eventos em segundo plano) não seria executada.
* O broker MQTT desconectaria o ESP32 por falha no envio de pacotes de controle (timeout de *keep-alive*).
* O dispositivo seria incapaz de responder instantaneamente a desconexões ou eventos na rede.

Ao usar `millis()`, criamos uma verificação condicional rápida (*polling*). O processador executa o loop em microsegundos, chamando a rotina de manutenção do MQTT continuamente e publicando os dados de forma assíncrona somente quando o tempo decorrido atinge o intervalo de 5 segundos.

### Por que JSON no payload?

Texto puro (`"73"`) funcionaria para o requisito mínimo, mas JSON permite:
- Múltiplos campos em uma única mensagem
- Identificação do grupo sem depender do tópico
- Compatibilidade direta com dashboards, bancos de dados e APIs

### Por que `randomSeed(analogRead(0))`? (Interação Física Bare Metal)

Computadores e microcontroladores são máquinas determinísticas: sem uma semente dinâmica, o gerador de números pseudo-aleatórios do ESP32 produziria **a exata mesma sequência** de dados a cada reinicialização.
Para resolver isso, realizamos a leitura de um pino analógico desconectado (`analogRead(0)`). Como o pino está eletricamente flutuante, ele atua como uma antena capturando o ruído térmico e eletromagnético do ambiente. Esse valor analógico físico e imprevisível inicializa a semente (`randomSeed`), garantindo a geração de dados verdadeiramente aleatórios.

### Por que WebSocket na porta 9001?

Browsers não podem abrir conexões TCP arbitrárias por segurança. O protocolo WebSocket (WS) é aceito por todos os browsers modernos e transporta MQTT por cima. O Mosquitto foi configurado com dois listeners: TCP 1883 (para ESP32) e WebSocket 9001 (para o dashboard HTML).

---

## 11. Dificuldades Encontradas e Soluções

| Dificuldade | Causa | Solução |
|---|---|---|
| ESP32 não encontra a rede Wi-Fi | Rede em 5 GHz | Conectar em rede 2.4 GHz ou usar hotspot do celular |
| Erro "Unable to verify flash chip" | ESP32 não entra em modo bootloader | Segurar botão BOOT durante o upload |
| Erro de símbolo no gnome-terminal | Conflito com versão snap | Usar `/usr/bin/gnome-terminal.wrapper` diretamente |
| Dashboard não recebe dados | Browser usa WebSocket, ESP32 usa TCP | Mosquitto configurado com dois listeners (1883 TCP + 9001 WS) |
| `pio` não encontrado no PATH | PlatformIO instalado em venv próprio | Usar caminho completo `~/.platformio/penv/bin/platformio` no Makefile |

---

## 12. Comandos de Referência Rápida

```bash
make help        # lista todos os comandos disponíveis
make broker      # sobe Mosquitto no Docker
make flash       # compila e grava o ESP32
make watch       # abre serial monitor + MQTT subscriber
make dashboard   # abre dashboard no browser
make logs        # logs do broker em tempo real
make stop        # para o broker
make test-pub    # publica mensagem de teste (sem ESP32)
```

```bash
# Assinar tópico manualmente no terminal
mosquitto_sub -h localhost -p 1883 -t "iot/grupo01/sensor" -v

# Publicar mensagem de teste manualmente
mosquitto_pub -h localhost -p 1883 -t "iot/grupo01/sensor" \
  -m '{"grupo":"01","valor":42,"envio":0}'
```
