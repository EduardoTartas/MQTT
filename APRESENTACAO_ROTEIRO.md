# Roteiro de Apresentação — Publicação de Dados Aleatórios via MQTT
Este documento serve como guia completo de divisão de slides e roteiro de fala para a apresentação do projeto (tempo estimado: 5 a 10 minutos).

---

## 📅 Visão Geral e Cronograma da Apresentação
* **Tempo Recomendado:** 7 a 8 minutos (equilíbrio ideal).
* **Divisão de Responsabilidades:** Se o grupo for de 2 ou 3 pessoas, as divisões recomendadas de fala estão sinalizadas no roteiro.

| Slide | Tópico | Tempo Estimado | Foco Principal |
| :--- | :--- | :--- | :--- |
| **Slide 1** | Capa e Introdução | 30 seg | Identificação e visão geral rápida do que é o projeto. |
| **Slide 2** | O Desafio & Objetivo | 45 seg | O que o sistema faz e quais requisitos atende. |
| **Slide 3** | Arquitetura do Sistema | 1 min | Explicar o fluxo de dados (ESP32 ➔ Broker ➔ Dashboard). |
| **Slide 4** | Tecnologias Utilizadas | 1 min | Justificar a escolha do ESP32, Docker, MQTT e WebSockets. |
| **Slide 5** | Requisitos & Diferenciais | 1 min | Mostrar o cumprimento dos requisitos e as melhorias feitas. |
| **Slide 6** | Decisões Técnicas Críticas | 1 min 30s | Explicar `millis()`, JSON, `randomSeed` e WebSockets. |
| **Slide 7** | Desafios & Soluções | 1 min | Compartilhar os problemas encontrados e como foram resolvidos. |
| **Slide 8** | **Demonstração Prática (Live Demo)** | 2 min | Mostrar o sistema funcionando em tempo real. |
| **Slide 9** | Conclusão | 30 seg | Fechamento e espaço para perguntas do professor/colegas. |

---

## 🛝 Estrutura de Slides e Script de Apresentação

### Slide 1: Capa do Projeto
* **Título Visual:** `Dispositivo IoT para Telemetria via MQTT`
* **Subtítulo:** `Publicação de Dados Aleatórios com ESP32 e Dashboard Web em Tempo Real`
* **Identificação:** Grupo 01 | Disciplina de Internet das Coisas.
* **Elementos Visuais Sugeridos:** Logos da instituição, ícone de Wi-Fi/MQTT ou imagem do ESP32.

> **🗣️ O que falar (Apresentador 1):**
> *"Olá a todos, nós somos o Grupo 01 e hoje vamos apresentar o nosso projeto prático de Internet das Coisas: um dispositivo de telemetria IoT baseado em ESP32 que gera e publica dados em tempo real utilizando o protocolo MQTT, acompanhado de um dashboard web interativo para visualização de dados."*

---

### Slide 2: O Desafio & Objetivos
* **Tópicos no Slide:**
  * Desenvolver um nó sensor (ESP32) para telemetria automatizada.
  * Conectar à rede sem fio local (Wi-Fi 2.4 GHz).
  * Gerar dados simulados (valores de 0 a 100) para representar leituras de sensores.
  * Enviar as mensagens periodicamente a cada 5 segundos.
  * Garantir a integridade da comunicação via broker MQTT.
* **Elementos Visuais:** Uma imagem ou diagrama conceitual simples de um chip ESP32 enviando ondas de rádio para uma nuvem/servidor.

> **🗣️ O que falar (Apresentador 1):**
> *"O nosso objetivo central foi criar um nó sensor IoT de ponta a ponta. Para simular as leituras de um sensor físico em laboratório, o dispositivo gera valores aleatórios entre 0 e 100 e faz o envio periódico desses dados a cada 5 segundos para um servidor centralizador, garantindo que o fluxo seja contínuo e sem perda de dados."*

---

### Slide 3: Arquitetura do Sistema (Fluxo de Dados)
* **Tópicos no Slide:**
  * **Publicador:** ESP32 DOIT DevKit V1 (via protocolo MQTT TCP - porta 1883).
  * **Broker:** Eclipse Mosquitto rodando isolado em container Docker.
  * **Assinantes (Clientes):**
    * Console do Terminal (`mosquitto_sub`) para depuração de rede.
    * Dashboard Web interativo (via WebSockets - porta 9001).
* **Elementos Visuais (Diagrama de Fluxo - Mermaid):**

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

* **Elementos Visuais Auxiliares (ASCII):**
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
          │  Assinante MQTT    │        │  WebSocket:9001       │
          └────────────────────┘        └──────────────────────┘
```

> **🗣️ O que falar (Apresentador 2):**
> *"Para estruturar o projeto, adotamos uma arquitetura clássica de Publish/Subscribe (Publicador/Assinante). Na ponta esquerda, temos o ESP32 atuando como publicador, que envia pacotes MQTT sob o protocolo TCP na porta 1883. No centro, o Broker Mosquitto gerencia o roteamento. E na ponta direita, temos dois assinantes simultâneos: o terminal de comandos e um Dashboard Web em tempo real que consome esses dados via WebSockets na porta 9001."*

---

### Slide 4: Tecnologias Utilizadas
* **Tópicos no Slide:**
  * **Hardware & Firmware:** ESP32 DevKit V1 + PlatformIO (C++ Arduino).
  * **Biblioteca MQTT:** `PubSubClient` (Nick O'Leary v2.8) — leve e rápida.
  * **Broker MQTT:** Eclipse Mosquitto 2.0 via Docker / Docker Compose.
  * **Visualização:** HTML5, CSS vanilla premium, MQTT.js e Chart.js no front-end.
* **Elementos Visuais:** Ícones ou logos das tecnologias (Docker, ESP32, MQTT, Chart.js, HTML5/CSS).

> **🗣️ O que falar (Apresentador 2):**
> *"Nós selecionamos ferramentas que tornam o ecossistema robusto e portátil. O firmware foi escrito usando PlatformIO para C++. Para o Broker MQTT, escolhemos o Eclipse Mosquitto, que é o padrão de mercado para IoT, rodando via Docker para facilitar a portabilidade do projeto. No cliente visual, criamos uma interface web em arquivo único usando HTML puro e Chart.js, permitindo que a demonstração seja executada em qualquer navegador sem configurações complexas."*

---

### Slide 5: Requisitos Atendidos & Diferenciais
* **Tópicos no Slide:**
  * **Obrigatórios ✅:**
    * Conexão Wi-Fi com monitoramento serial
    * Comunicação ativa com Broker MQTT
    * Geração de números aleatórios pseudo-imprevisíveis
    * Envio estrito a cada 5 segundos
  * **Diferenciais do Grupo 01 ✅:**
    * **Payload JSON:** Dados estruturados com metadados adicionais.
    * **Resiliência (Auto-Reconexão):** Tratamento automático de quedas de Wi-Fi e conexão MQTT.
    * **Contador de mensagens:** Identificação visual de mensagens perdidas.
    * **Gráfico Dinâmico:** Dashboard moderno com rolagem de dados em tempo real.

> **🗣️ O que falar (Apresentador 1 ou 3):**
> *"Além de cumprir todos os requisitos mínimos propostos, adicionamos diferenciais importantes. Em vez de enviar texto puro, estruturamos os dados em JSON contendo o identificador do grupo e um contador incremental de envios, o que permite diagnosticar perdas de pacotes. Também implementamos rotinas de reconexão automática: caso o Wi-Fi ou o Broker fiquem offline momentaneamente, o ESP32 restabelece a conexão sozinho assim que o sinal retorna."*

---

### Slide 6: Paradigmas e Decisões Técnicas (Bare Metal & Arquitetura)
* **Tópicos no Slide:**
  * **Paradigma de Programação:** **Bare Metal Super Loop** com **Multitarefa Cooperativa**.
  * **Temporização Não-Bloqueante (`millis()`):** Em sistemas mono-thread (bare-metal), o `delay(5000)` congela a CPU. O `millis()` implementa um escalonador por *polling*, permitindo concorrência e mantendo o `mqttClient.loop()` ativo (evita quedas por timeout).
  * **Acesso Direto ao Hardware (ADC):** Uso de `analogRead(0)` em pino flutuante para ler o ruído térmico/eletromagnético real do conversor analógico-digital (ADC) para alimentar o `randomSeed`.
  * **FreeRTOS vs. Bare Metal:** Embora o ESP32 rode o FreeRTOS sob o framework Arduino, projetamos o código seguindo as melhores práticas de loops bare-metal clássicos para reduzir a complexidade e o overhead de contexto.
* **Elementos Visuais:** Trechos do código em destaque (a lógica do `millis()` comparada a um fluxograma de Super Loop Cooperativo).

> **🗣️ O que falar (Apresentador 2 ou 3):**
> *"No Slide 6, destacamos os paradigmas de arquitetura de firmware adotados. Embora o ESP32 possua um sistema operacional de tempo real rodando por baixo — o FreeRTOS —, nós desenvolvemos a aplicação seguindo o paradigma clássico de **Bare Metal Super Loop** com **Multitarefa Cooperativa**.
> Em sistemas bare-metal puros, não temos um gerenciador de tarefas preemptivo para pausar processos. Por isso, a escolha de usar `millis()` em vez de `delay()` é crucial: ela atua como um temporizador não-bloqueante por pooling. Se usássemos `delay`, a CPU travaria por 5 segundos, impedindo o processamento do protocolo MQTT de segundo plano e desconectando o cliente.
> Além disso, exploramos o comportamento físico do hardware: ao ler o pino analógico desconectado (`analogRead`), estamos capturando o ruído térmico do conversor ADC para gerar números pseudo-aleatórios verdadeiramente imprevisíveis a cada reinicialização."*

---

### Slide 7: Desafios Encontrados & Soluções
* **Tópicos no Slide:**
  * **Rede Wi-Fi:** O ESP32 não opera em 5 GHz ➔ *Solução:* Rede dedicada de 2.4 GHz ou Roteador do Celular.
  * **Bloqueio do Navegador:** Erro na comunicação TCP direta ➔ *Solução:* Configuração do listener Mosquitto com protocolo WebSocket.
  * **Falha de Gravação:** Erro do bootloader do ESP32 ➔ *Solução:* Pressionar o botão BOOT fisicamente ao iniciar a escrita.
* **Elementos Visuais:** Tabela com problemas e respectivas soluções de forma clara e legível.

> **🗣️ O que falar (Apresentador 1 ou 3):**
> *"Durante o desenvolvimento, enfrentamos alguns desafios típicos de hardware e redes. O primeiro foi o Wi-Fi do laboratório rodar em 5 GHz, frequência não suportada pelo ESP32 DevKit V1. Contornamos configurando o ESP32 no ponto de acesso 2.4 GHz de um celular corporativo. Outra dificuldade foi fazer o Dashboard Web receber as mensagens; descobrimos que navegadores modernos barram conexões socket TCP brutas por segurança, o que resolvemos adicionando a porta de WebSockets ao broker."*

---

### Slide 8: Demonstração Prática (Roteiro da Live Demo)
* **Tópicos no Slide:**
  * Apresentação em tempo real do sistema em funcionamento.
  * Tópico monitorado: `iot/grupo01/sensor`.
  * Visualização simultânea: Terminal Linux + Monitor Serial + Dashboard Gráfico.
* **Elementos Visuais:** Mostrar a tela do computador onde estão o terminal e o navegador lado a lado.

> **🗣️ O que falar (Qualquer membro / Operador da máquina):**
> *(Ver Roteiro da Demonstração Prática abaixo para o passo a passo exato).*

---

### Slide 9: Conclusão & Perguntas
* **Tópicos no Slide:**
  * Resumo das conquistas do projeto.
  * Escalabilidade: Como esse modelo pode ser expandido para sensores industriais reais (temperatura, umidade, vibração).
  * Agradecimentos e espaço para perguntas.
* **Elementos Visuais:** Ícone de perguntas/balão de conversa.

> **🗣️ O que falar (Todos):**
> *"Concluímos que a arquitetura MQTT com ESP32 se provou extremamente leve, confiável e rápida para telemetria IoT. Com poucas modificações, poderíamos conectar sensores de temperatura ou umidade reais no lugar dos dados aleatórios. Agradecemos a atenção de todos e abrimos espaço para perguntas ou considerações do professor."*

---

## 🛠️ Roteiro de Execução da Demonstração Prática (Live Demo)

Para que a apresentação prática seja dinâmica e sem imprevistos, siga esta sequência exata de comandos e ações:

### 1. Preparação (Antes da apresentação começar)
* Deixe o ESP32 conectado no cabo USB da sua máquina.
* Verifique se o IP do seu computador na rede Wi-Fi está correto e configurado no `src/main.cpp` e no arquivo `APRESENTACAO.md` (no exemplo, `10.42.0.164`).
* Deixe o terminal aberto no diretório do projeto.

### 2. Passo 1: Inicializar o Broker (Docker)
No terminal, execute:
```bash
make broker
```
* **O que explicar para a banca:** *"Estamos subindo nosso broker MQTT Mosquitto local em um container Docker, isolando-o de qualquer serviço na máquina. Ele já está configurado para expor a porta de telemetria TCP 1883 e a porta WebSockets 9001."*

### 3. Passo 2: Mostrar o ESP32 Inicializando e Enviando Dados (Monitor Serial)
Abra a conexão com o monitor serial do ESP32 executando:
```bash
make watch
```
* **O que mostrar:** Mostre o ESP32 conectando ao Wi-Fi local, imprimindo os pontos de progresso, obtendo o endereço IP e fazendo o handshake MQTT inicial com sucesso.
* **O que explicar para a banca:** *"Aqui no monitor serial podemos ver o ESP32 inicializando. Ele estabeleceu a conexão Wi-Fi de 2.4 GHz, recebeu um IP local da rede, conectou-se ao nosso broker Docker e começou a enviar os dados a cada 5 segundos contendo o valor aleatório gerado e o contador incremental."*

### 4. Passo 3: Mostrar as Mensagens no Terminal (mosquitto_sub)
Na janela paralela aberta pelo comando `make watch` (ou executando manualmente `mosquitto_sub -h localhost -p 1883 -t "iot/grupo01/sensor" -v`):
* **O que mostrar:** As mensagens brutas em formato JSON chegando na tela.
* **O que explicar para a banca:** *"Esta janela representa um cliente genérico do broker rodando via terminal de comando. Ela está assinando o tópico `iot/grupo01/sensor` e exibe em tempo real o payload JSON que o ESP32 publica. Podemos ver a estrutura contendo os campos: `grupo`, `valor` e `envio`."*

### 5. Passo 4: O Dashboard Web (O diferencial visual)
No terminal, digite o comando abaixo para abrir a página web:
```bash
make dashboard
```
* Digite o IP correto do seu computador no campo de input do Dashboard e clique no botão **Conectar**.
* **O que mostrar:** O status mudar de vermelho (Desconectado) para verde (Conectado). Os cards com "Último Valor", "Mensagens recebidas" e "Grupo" atualizando de 5 em 5 segundos, e a linha do gráfico Chart.js subindo e descendo dinamicamente.
* **O que explicar para a banca:** *"Este é o nosso Dashboard Web em tempo real. Ele se conecta via WebSockets ao broker Mosquitto. À medida que o ESP32 publica, a página atualiza instantaneamente o gráfico dinâmico de telemetria e alimenta um log de mensagens histórico na parte inferior. Tudo isso de forma assíncrona e visual."*

### 6. Passo 5 (Demonstração de Resiliência - Opcional, mas impressionante)
* Remova o cabo USB do ESP32 por 6 segundos e conecte novamente (ou desligue o Wi-Fi do seu computador brevemente).
* **O que mostrar:** A interface web parando de receber dados, e assim que o ESP32 reinicia/reconecta, a transmissão retorna do ponto onde parou sem precisar reiniciar o docker ou recarregar a página web.
* **O que explicar para a banca:** *"Para simular uma falha de campo, simulamos uma perda de conexão. O ESP32 identificou a perda de link, manteve as rotinas de segurança rodando no loop secundário via `millis()` e restabeleceu a comunicação de forma totalmente automatizada."*
