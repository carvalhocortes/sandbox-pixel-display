# Pixel Display

Firmware PlatformIO para um painel WS2812 de 16×16 conectado a um ESP8266 NodeMCU. O painel alterna GIFs armazenados em um cartão SD com a hora e a data lidas de um RTC, além de aceitar atualizações OTA via Wi-Fi.

## Funcionalidades

- Reprodução aleatória de GIFs em `/gifs`, sem repetir imediatamente o último arquivo.
- Hora em duas linhas com dígitos 3×5: hora e minutos.
- Data em duas linhas: `DD  MM` e `AAAA`.
- Indicadores de inicialização `RTC OK` e `RTC NOK` no painel.
- Sincronização do RTC com a data/hora usadas na compilação.
- Mensagens no monitor serial para a imagem, hora e data exibidas.
- Atualização OTA pelo PlatformIO depois da primeira instalação via USB.

## Hardware e ligações

| Função | NodeMCU |
|---|---|
| Dados WS2812 | D4 |
| Chip Select SD | D3 |
| RTC SDA | D2 |
| RTC SCL | D1 |
| Alimentação/GND | VCC/GND comuns |

O módulo HW-111 usa um RTC DS1307 no endereço I²C `0x68`. O endereço `0x50` é a EEPROM presente no mesmo módulo.

### Adaptação elétrica do RTC

O módulo RTC possui resistores de pull-up do barramento I²C ligados à alimentação da placa. Como o ESP8266 trabalha com GPIOs de 3,3 V, as linhas `SDA` e `SCL` não devem receber pull-up em 5 V.

Nesta montagem, foi feita uma adaptação resistiva nas linhas `SDA` e `SCL` para adequar os níveis do módulo RTC ao ESP8266. Essa adaptação é necessária para evitar leituras intermitentes ou a perda do RTC após reinicializações. Os valores e o esquema exatos dos resistores dependem da placa utilizada e devem ser mantidos conforme a montagem validada.

## Estrutura do código

```text
src/main.cpp              Bootstrap mínimo do Arduino
src/DisplayApplication.cpp Composição e coordenação da aplicação
src/DisplayScheduler.cpp   Alternância entre imagem, hora e data
src/GifPlayer.cpp          Decodificação GIF e seleção aleatória
src/LedMatrix.cpp          Buffer WS2812 e mapeamento serpentino
src/ClockRenderer.cpp      Renderização dos dígitos 3×5
src/RtcClock.cpp           Acesso e sincronização do RTC
src/OtaService.cpp         Wi-Fi e ArduinoOTA
src/DisplayLogger.cpp      Logs do conteúdo exibido
src/FilenameFunctions.cpp  Callbacks de arquivos do SD/GIF
include/DisplayConfig.h    Pinos, dimensões e temporizações
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
[TIME] 17:42
[DATE] 08/08 2026
```

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

## Sincronização do RTC

O firmware usa `__DATE__` e `__TIME__` durante o build. Portanto:

1. Confirme o horário do computador.
2. Faça o Build.
3. Faça o Upload logo em seguida.

O RTC é sincronizado no boot. A biblioteca usada é `RTClib`; o firmware utiliza a interface `RTC_DS1307` no endereço `0x68`.

Se a inicialização falhar, o monitor serial mostrará `RTC inicializacao: NOK` e o painel exibirá `RTC NOK`. Quando a comunicação estiver funcionando, serão mostrados `RTC inicializacao: OK`, a sincronização com o horário do computador e `RTC OK` no painel.

## Desenvolvimento

Antes de commitar:

```sh
git diff --check
pio run -e nodemcuv2
```

Não versione `.pio/`, `platformio.local.ini`, senhas ou outros arquivos de configuração local.
