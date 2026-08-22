# Tela do relógio de Fibonacci

## Escopo

Adicionar um novo modo navegável ao painel WS2812 de 16×16 que represente a
hora do RTC usando cinco quadrados com os valores `5`, `3`, `2`, `1` e `1`.
O modo deve conviver com GIFs, hora digital, data e dia da semana sem alterar os
contratos existentes de hardware, RTC ou botões.

## Contrato visual

- Horas usam a hora em formato de 12 horas; `00:xx` é representado como `12`.
- Minutos são truncados para blocos de cinco: `37` é exibido como `35`.
- Cada quadrado é selecionado quando seu valor participa da soma das horas,
  dos minutos ou de ambas as grandezas.
- Vermelho representa horas, verde representa minutos e laranja representa um
  quadrado usado simultaneamente pelos dois valores.
- Quadrados não selecionados permanecem em uma intensidade muito baixa para
  preservar o mapa visual sem competir com a leitura.
- Os quadrados encostam uns nos outros; não há linhas ou pixels pretos entre os
  valores que compõem o desenho.
- O layout segue a composição clássica: o quadrado `5` fica à esquerda, o `3`
  acima à direita, o `2` abaixo dele e os dois quadrados `1` ficam ao lado do
  `2`.
- As dimensões visuais são `8×8` para o `5`, `4×4` para o `3`, `2×4` para o
  `2` e `2×2` para cada quadrado `1`.
- O conjunto ocupa as colunas centrais do painel, sem deslocamento vertical ou
  horizontal intencional.
- A seleção dos quadrados é determinística, começando pelos maiores valores e
  usando os dois quadrados de valor `1` apenas quando necessário.

## Navegação

O novo modo entra entre a hora e a data:

`GIFs → hora → Fibonacci → data → dia da semana → GIFs`

`LEFT` e `RIGHT` continuam navegando entre modos. `UP`, `DOWN` e `SELECT` não
ganham ações específicas no modo Fibonacci.

## Critérios de aceite

1. A branch é criada a partir da `main` local.
2. O modo Fibonacci aparece na sequência de navegação e renderiza o horário do
   RTC sem bloquear o loop da aplicação.
3. A soma dos quadrados vermelhos corresponde às horas e a soma dos quadrados
   verdes corresponde aos blocos de cinco minutos; laranja conta para ambos.
4. A tela é redesenhada somente quando a hora ou o bloco de cinco minutos muda.
5. O build PlatformIO do ambiente `nodemcuv2` continua passando.
6. O README descreve a nova tela e seus controles.

## Riscos e decisões

- A leitura depende de cores, portanto o README documenta o código de cores e
  a redução dos minutos para múltiplos de cinco.
- A validação visual final depende do painel físico; o build confirma
  compilação, mas não substitui a inspeção dos LEDs e da orientação serpentina.
- Não haverá edição de data/hora nem ajuste de brilho dentro desse modo; essas
  responsabilidades permanecem nos modos existentes.
