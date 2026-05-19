PIO     := $(HOME)/.platformio/penv/bin/platformio
BROKER  := mqtt-broker
TOPICO  := iot/grupo01/sensor
BROKER_IP := localhost

.PHONY: all broker flash monitor sub watch stop logs deps dashboard help

## Fluxo completo: sobe broker + flash + abre janelas
all: broker flash watch

## Sobe broker Mosquitto no Docker
broker:
	@echo "→ Subindo broker MQTT..."
	cd $(BROKER) && docker compose up -d
	@echo "→ Broker rodando em localhost:1883"

## Compila e grava no ESP32
flash:
	@echo "→ Compilando e gravando ESP32..."
	$(PIO) run --target upload

## Monitor serial do ESP32
monitor:
	$(PIO) device monitor --baud 115200

## Assina tópico MQTT e imprime payloads
sub:
	mosquitto_sub -h $(BROKER_IP) -p 1883 -t "$(TOPICO)" -v

## Abre abas gnome-terminal: serial + MQTT simultâneos
watch:
	@echo "→ Abrindo janelas de monitoramento..."
	/usr/bin/gnome-terminal.wrapper \
		--tab --title="Serial ESP32" \
			-- bash -c "$(PIO) device monitor --baud 115200; exec bash" \
		--tab --title="MQTT Sub" \
			-- bash -c "sleep 1 && mosquitto_sub -h $(BROKER_IP) -p 1883 -t '$(TOPICO)' -v; exec bash"

## Abre dashboard no browser
dashboard:
	xdg-open dashboard/index.html

## Para o broker
stop:
	cd $(BROKER) && docker compose down

## Logs do broker em tempo real
logs:
	cd $(BROKER) && docker compose logs -f mosquitto

## Instala dependências do sistema
deps:
	sudo apt-get install -y mosquitto-clients tmux

## Publica mensagem de teste no broker
test-pub:
	mosquitto_pub -h $(BROKER_IP) -p 1883 -t "$(TOPICO)" \
		-m '{"grupo":"01","valor":99,"envio":0}'

help:
	@echo ""
	@echo "Targets disponíveis:"
	@echo "  make all        → broker + flash + watch"
	@echo "  make broker     → sobe Mosquitto no Docker"
	@echo "  make flash      → compila e grava ESP32"
	@echo "  make monitor    → serial monitor"
	@echo "  make sub        → assina tópico MQTT"
	@echo "  make watch      → abre abas: serial + mqtt"
	@echo "  make stop       → para o broker"
	@echo "  make logs       → logs do broker"
	@echo "  make test-pub   → publica msg de teste"
	@echo "  make dashboard  → abre dashboard no browser"
	@echo "  make deps       → instala mosquitto-clients"
	@echo ""
