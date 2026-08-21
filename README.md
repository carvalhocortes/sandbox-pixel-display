# Pixel Display

Firmware PlatformIO para um painel WS2812 de 16×16 conectado a um ESP8266 NodeMCU. O painel mostra GIFs armazenados em um cartão SD, a hora, a data e o dia da semana lidos de um RTC, com navegação manual pelos botões e atualizações OTA via Wi-Fi.

## Funcionalidades

- Reprodução dos GIFs em `/gifs`, com troca manual pelos botões e repetição do GIF atual.
- Hora em duas linhas com dígitos 3×5: hora e minutos.
- Data em duas linhas: `DD  MM` e `AAAA`.
- Dia da semana em formato abreviado: `SEG`, `TER`, `QUA`, `QUI`, `SEX`, `SAB` ou `DOM`.
- Menu manual entre GIFs, hora, data e dia da semana.
- GIF favorito salvo na memória persistente do ESP8266 e restaurado após reiniciar.
- Brilho global ajustável no modo dia da semana e restaurado após reiniciar.
- Indicadores de inicialização `RTC OK` e `RTC NOK` no painel.
- Sincronização do RTC com a data/hora usadas na compilação.
- Ajuste do RTC por comando no monitor serial.
- Leitura diagnóstica dos cinco botões analógicos pelo monitor serial.
- Mensagens no monitor serial para a imagem, hora, data e dia da semana exibidos.
- Atualização OTA pelo PlatformIO depois da primeira instalação via USB.

## Hardware e ligações

| Função | NodeMCU |
|---|---|
| Dados WS2812 | D4 |
| Chip Select SD | D3 |
| RTC SDA | D2 |
| RTC SCL | D1 |
| Módulo de botões analógicos | A0 |
| Alimentação/GND | VCC/GND comuns |

O módulo HW-111 usa um RTC DS1307 no endereço I²C `0x68`. O endereço `0x50` é a EEPROM presente no mesmo módulo.

### Adaptação elétrica do RTC

O módulo RTC possui resistores de pull-up do barramento I²C ligados à alimentação da placa. Como o ESP8266 trabalha com GPIOs de 3,3 V, as linhas `SDA` e `SCL` não devem receber pull-up em 5 V.

Nesta montagem, foi feita uma adaptação resistiva nas linhas `SDA` e `SCL` para adequar os níveis do módulo RTC ao ESP8266. Essa adaptação é necessária para evitar leituras intermitentes ou a perda do RTC após reinicializações. Os valores e o esquema exatos dos resistores dependem da placa utilizada e devem ser mantidos conforme a montagem validada.

### Diagrama do circuito atual

O diagrama abaixo representa as ligações digitais e do RTC já definidas. O
módulo de cinco botões usa uma rede resistiva e está ligado à entrada analógica
`A0`, com `VCC` e `GND` comuns. Os valores individuais ainda são descobertos
durante o diagnóstico do firmware.

```text
                                      +----------------------+
                                      |  Matriz WS2812 16x16 |
                                      |  DIN                 |
                                      +----^-----------------+
                                           |
                                           | D4
                                           |
+----------------------+                   |
| ESP8266 NodeMCU      |                   |
|                      |                   |
| D3  -----------------+-----------------------> CS SD
|                      |                   |
| D5  ---------------------------------------> SCK SD
| D6  <--------------------------------------- MISO SD
| D7  ---------------------------------------> MOSI SD
|                      |                   |
| D2  -------- SDA ---[adaptação resistiva]--- SDA RTC
| D1  -------- SCL ---[adaptação resistiva]--- SCL RTC
|                      |                   |
| VCC -----------------+----------------------> VCC dos módulos
| GND -----------------+----------------------> GND comum
+----------------------+                   |
                                           |
                                      +----v-------------+
                                      | Cartão/módulo SD |
                                      +------------------+

                                      +------------------+
                                      | Módulo RTC HW-111|
                                      | DS1307: 0x68     |
                                      | EEPROM: 0x50     |
                                      +------------------+
```

No NodeMCU, o cartão SD usa o barramento SPI de hardware: `D5` (SCK), `D6`
(MISO), `D7` (MOSI) e `D3` (CS). O RTC usa I²C em `D2` (SDA) e `D1` (SCL).
As linhas I²C devem permanecer com pull-ups compatíveis com 3,3 V; não ligar
pull-ups de 5 V diretamente aos GPIOs do ESP8266.

## Estrutura do código

```text
src/main.cpp              Bootstrap mínimo do Arduino
src/DisplayApplication.cpp Composição e coordenação da aplicação
src/DisplayScheduler.cpp   Alternância entre imagem, hora, data e dia da semana
src/GifPlayer.cpp          Decodificação GIF e seleção manual/persistente
src/LedMatrix.cpp          Buffer WS2812 e mapeamento serpentino
src/ClockRenderer.cpp      Renderização dos dígitos e letras 3×5
src/RtcClock.cpp           Acesso e sincronização do RTC
src/AnalogButtonReader.cpp Leitura diagnóstica do módulo de botões em A0
src/BrightnessStore.cpp    Persistência do brilho global
src/OtaService.cpp         Wi-Fi e ArduinoOTA
src/DisplayLogger.cpp      Logs do conteúdo exibido
src/FilenameFunctions.cpp  Callbacks de arquivos do SD/GIF
include/DisplayConfig.h    Pinos, dimensões e calibração dos botões
include/AnalogButtonReader.h Interface do leitor de botões analógicos
include/BrightnessStore.h Interface da persistência do brilho
```

## Preparação no VS Code

1. Instale a extensão PlatformIO.
2. Abra esta pasta como projeto.
3. O ambiente usado é `nodemcuv2`.
4. Copie os GIFs da pasta `img/` (ou outros arquivos compatíveis) para a pasta `/gifs` do cartão SD.

As bibliotecas são baixadas automaticamente conforme `platformio.ini`:

- GifDecoder
- FastLED
- AnimatedGIF
- RTClib

## Primeiro upload via USB

```sh
pio run -e nodemcuv2
pio run -e nodemcuv2 -t upload
pio device monitor -b 115200
```

Também é possível usar os botões **Build**, **Upload** e **Monitor** da extensão PlatformIO no VS Code.

O monitor deve mostrar mensagens como:

```text
[IMAGE] /gifs/nome-do-arquivo.gif
[TIME] 17:42:01
[DATE] 08/08/2026
[WEEKDAY] DOM
```

### Diagnóstico dos botões

Depois do boot, abra o monitor serial em `115200` baud. O firmware mede o valor
de repouso de `A0` e imprime uma linha sempre que detectar um pressionamento
estável:

```text
Botoes analogicos em A0; repouso ADC=1023
Pressione um botao para identificar o valor ADC
Botao UP pressionado; ADC=155
Botoes analogicos liberados; ADC=1022
```

Os intervalos atualmente identificados são:

| Botão | Faixa ADC |
|---|---:|
| LEFT | 0–10 |
| UP | 150–160 |
| DOWN | 340–350 |
| RIGHT | 525–535 |
| SELECT | 770–780 |

O número `ADC` é a leitura bruta do ESP8266, entre `0` e `1023`. Se uma leitura
ficar fora das faixas, o terminal mostrará `DESCONHECIDO` para permitir ajustar
os intervalos posteriormente.

### Menu pelos botões

O modo mostrado não muda sozinho. O menu inicia em GIFs e segue a sequência
`GIFs → hora → data → dia da semana → GIFs`:

| Botão | Ação |
|---|---|
| LEFT | modo anterior na sequência |
| RIGHT | próximo modo na sequência |
| UP | GIF anterior no modo GIFs; aumenta o brilho no modo dia |
| DOWN | próximo GIF no modo GIFs; diminui o brilho no modo dia |
| SELECT | salva o GIF atual como favorito, somente no modo GIFs |

No modo dia da semana, `SELECT` não faz nada. `UP` e `DOWN` ajustam o brilho
global em passos de 5%, entre 5% e 100%, e cada alteração é salva imediatamente
na EEPROM emulada do ESP8266. O valor padrão é 50%. O terminal informa o
percentual atual após cada alteração. O `SELECT` grava
o caminho do GIF na EEPROM emulada do ESP8266; ao reiniciar, esse arquivo volta a
ser exibido automaticamente, desde que continue presente no cartão SD.

No modo data, `SELECT` inicia a edição do dia. Cada novo `SELECT` avança para o
mês, depois para o ano e, por fim, salva a data. No modo hora, a sequência é
`minutos → horas → salvar`. Durante a edição, `UP` aumenta e `DOWN` diminui o
campo selecionado. Os valores são limitados automaticamente ao calendário e às
faixas válidas de hora. `LEFT` ou `RIGHT` cancelam uma edição em andamento e
navegam para outro modo. O relógio normal usa branco; somente o campo em edição
fica verde claro e pisca duas vezes por segundo. Quando a gravação é concluída,
todo o conteúdo pisca duas vezes em vermelho antes de voltar ao branco. Botões
pressionados durante essa confirmação visual são ignorados.

## Configuração Wi-Fi local

Copie o exemplo:

```sh
cp platformio.local.ini.example platformio.local.ini
```

Edite o arquivo local:

```ini
[env:nodemcuv2]
build_flags =
    -DWIFI_SSID=\"NomeDaRede\"
    -DWIFI_PASSWORD=\"SenhaDaRede\"
```

`platformio.local.ini` está no `.gitignore` e não deve ser enviado ao GitHub.

## Upload OTA

Faça primeiro o upload via USB. Depois que o monitor mostrar o IP do ESP, adicione ao `platformio.local.ini`:

```ini
[env:nodemcuv2]
build_flags =
    -DWIFI_SSID=\"NomeDaRede\"
    -DWIFI_PASSWORD=\"SenhaDaRede\"
upload_protocol = espota
upload_port = 192.168.0.123
```

Substitua o IP pelo endereço mostrado no monitor e execute novamente **Upload** no PlatformIO. Se o IP mudar, atualize `upload_port`.

Se a rede estiver indisponível ou as credenciais estiverem incorretas, o firmware aguardará até 15 segundos pela conexão e continuará funcionando sem OTA.

## Sincronização do RTC

O firmware usa `__DATE__` e `__TIME__` durante o build e grava o timestamp da
compilação no NVRAM interno do DS1307. Portanto:

1. Confirme o horário do computador.
2. Faça o Build.
3. Faça o Upload logo em seguida.

Quando o timestamp do firmware for diferente do último timestamp salvo, o
RTC é sincronizado uma vez com o horário da compilação. Em reinicializações
normais, o horário salvo no RTC é preservado. Assim, um novo firmware corrige
uma hora antiga, mas o mesmo firmware não reajusta o RTC a cada boot.
Quando a comunicação estiver funcionando, o monitor mostra a data e hora atual
lidas do RTC. A biblioteca usada é `RTClib`; o firmware utiliza a interface
`RTC_DS1307` no endereço `0x68`.

Se a inicialização falhar, o monitor serial mostrará `RTC inicializacao: NOK` e o painel exibirá `RTC NOK`. Quando a comunicação estiver funcionando, serão mostrados `RTC inicializacao: OK`, a sincronização com o horário do computador e `RTC OK` no painel.

### Ajuste pelo monitor serial

O monitor serial não envia automaticamente a hora do computador para o ESP8266.
Para ajustar o RTC, copie a hora atual do computador e envie uma linha no formato:

```text
SYNC 2026-08-09 21:30:00
```

O formato é `SYNC AAAA-MM-DD HH:MM:SS`. O firmware responde com o horário gravado
e preserva esse ajuste nos próximos reinícios. Use `STATUS` para consultar o
horário atual e `HELP` para listar os comandos.

Para enviar a hora local automaticamente, feche o monitor serial e execute no
computador:

```sh
python tools/sync_rtc.py --port /dev/cu.usbserial-XXXX
```

Substitua a porta pelo dispositivo do NodeMCU. O script envia a hora atual e
aguarda a confirmação `RTC ajustado para`.

## Desenvolvimento

Antes de commitar:

```sh
git diff --check
pio run -e nodemcuv2
```

Não versione `.pio/`, `platformio.local.ini`, senhas ou outros arquivos de configuração local.
