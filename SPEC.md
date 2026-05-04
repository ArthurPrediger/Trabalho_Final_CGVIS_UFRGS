# Especificação da Implementação

> [!CAUTION]
> - Você <ins>**não pode utilizar ferramentas de IA para escrever esta
>   especificação**</ins>

## Integrantes da dupla

- **Aluno 1 - Nome**: Arthur Rambo Prediger
- **Aluno 1 - Cartão UFRGS**: 00334521

- **Aluno 2 - Nome**: Taylor Souza Frutuoso da Costa
- **Aluno 2 - Cartão UFRGS**: 00213957

## Detalhes do que será implementado

- **Título do trabalho**: Autorama Arcade
- **Parágrafo curto descrevendo o que será implementado**: será implementado um jogo que simula o funcionamento de um autorama (automodelismo de fenda) inspirado nas funcionalidades arcade do estágio Slot-Car Derby do jogo Mario Party Superstars. 

## Especificação visual

### Vídeo - Link

> [!IMPORTANT]
> - Coloque aqui um link para um vídeo que mostre a aplicação gráfica
>   de referência que você vai implementar. **Sua implementação deverá
>   ser o mais parecido possível com o que é mostrado no vídeo (mais
>   detalhes abaixo).**
> - **Você não pode escolher como referência: (1) algum trabalho realizado
>   por outros alunos desta disciplina, em semestres anteriores. (2) Minecraft.**
> - Por exemplo, você pode colocar um vídeo de um jogo que você gosta,
>   e seu trabalho final será uma re-implementação do jogo.
> - O vídeo pode ser um link para YouTube, Google Drive, ou arquivo mp4 dentro
>   do próprio repositório. Mas, garanta que qualquer um tenha
>   permissão de acesso ao vídeo através deste link.

Link 1, mecânicas e interações pretendidas: https://www.youtube.com/watch?v=iQR6Ru8bRtg

Link 2, visual pretendido: https://www.youtube.com/watch?v=GAPWfClVFJI

### Vídeo - Timestamp

> [!IMPORTANT]
> - Coloque aqui um **intervalo de ~30 segundos** do vídeo acima, que
>   será a base de comparação para avaliar se o seu trabalho final
>   conseguiu ou não reproduzir a referência.

- **Timestamp inicial 1**: 00:00
- **Timestamp final 1**: 00:30
- **Timestamp inicial 2**: 01:55
- **Timestamp final 2**: 02:25

### Imagens

> [!IMPORTANT]
> - Coloque aqui **três imagens** capturadas do vídeo acima, que você
>   irá usar como ilustração para as explicações que vêm abaixo.

<img width="1600" height="900" alt="image" src="https://github.com/user-attachments/assets/00f439b1-f3ee-401f-b02f-c8af2b503415" />

<img width="1567" height="655" alt="image" src="https://github.com/user-attachments/assets/34e0b688-05dd-4510-8fe6-2cdf4083f72a" />

<img width="1594" height="745" alt="image" src="https://github.com/user-attachments/assets/d8da57d9-d707-4ff9-aa49-431e7d5c7dba" />


## Especificação textual

Para cada um dos requisitos abaixo (detalhados no [Enunciado do Trabalho final - Moodle](https://moodle.ufrgs.br/mod/assign/view.php?id=6018620)), escreva um parágrafo **curto** explicando como este requisito será atendido, apontando itens específicos do vídeo/imagens que você incluiu acima que atendem estes requisitos.

### Malhas poligonais complexas
A aplicação conterá malhas de carros que serão complexas, semelhantes aos das imagens 2 e 3, com os modelos exatos a depender do que for encontrado em repositórios online. A geometria da pista e eventuais objetos decorando as laterais dela também terão complexidade adequada para que apresentem curvas suaves, dando ênfase principal aos carros e pista.

### Transformações geométricas controladas pelo usuário
O usuário poderá controlar a posição do seu carro acelerando e freiando de forma que ele percorra o trajeto da pista de forma semelhante ao vídeo do primeiro link. A rotação também será controlada pelo usuário de forma indireta conforme o carro precise se manter alinhado com o trajeto pré-determinado. Na versão do autorama em Mario Party, cada carro se mantém em sua faixa, uma nova mecânica de troca de faixas que desloca o carro lateralmente também visa ser implementada.

### Diferentes tipos de câmeras
O estilo de câmera padrão utilizado ao longo da corrida será semelhante ao do primeiro vídeo, com a câmera regulando seu afastamento e posição em relação a pista para manter todos os jogadores visíveis em tela. Como no vídeo, esse estilo de câmera se mantém mais próximo da pista se os jogadores estiverem andando próximos/aglomerados. Um segundo estilo de câmera será adicionado numa espécie de replay, que permitirá o jogador ver a última corrida a partir de uma câmera que segue qualquer um dos carros e pode ser rotacionada ao redor dele. 

### Instâncias de objetos
Múltiplas instâncias de árvores, placas ou outros objetos decorativos serão adicionados nas laterais da pista, semelhantemente ao segundo vídeo. Modelos exatos de objetos dependerão do que for encontrado online.  

### Testes de intersecção
Devido a inclusão da mecânica exclusiva de troca de faixa dinâmica que o usuário escolhe para o carro percorrer, serão testadas intersecções entre os carros que estão utilizando as mesmas faixas. Hitboxes simplificadas serão utilizadas para cada carro e a intenção é que o carro que colide na traseira de outro sofra um atraso maior na retomada de velocidade.  

### Modelos de Iluminação em todos os objetos
A respeito da iluminação, será considerada uma fonte de luz direcional que representará o Sol e mais um fator de luz ambiente que se assemelhe com a terceira imagem adicionada acima. Será utilizado um shader parametrizável que possibilite a representação de materias dos seguintes tipos: metais, pinturas automotivas, borrachas, plásticos, madeiras e folhas. 

### Mapeamento de texturas em todos os objetos
Serão utilizados modelos com mapeamento UV e texturas de albedo e normais já pré-computados para os carros, pista e objetos nas laterais do cenário. Caso os modelos adquiridos de forma online possuam outras texturas (especular, rugosidade, oclusão de ambiente) essas também serão utilizadas. 

### Movimentação com curva Bézier cúbica
A movimentação dos carros sobre a pista será baseada em curvas de bézier cúbicas combinadas para formar o trajeto total da pista.

### Animações baseadas no tempo ($\Delta t$)
A rotação das rodas dos carros serão baseadas no tempo. Haverão placas de sinalização na pista, semelhantes às do primeiro vídeo, que também serão animadas baseadas em tempo. Será implementado um efeito visual com partículas pra sinalizar a largada dos carros. 

## Limitações esperadas

> [!IMPORTANT]
> - Coloque aqui uma lista de detalhes visuais ou de interação que
>   aparecem no vídeo e/ou imagens acima, mas que você **não pretende
>   implementar** ou que você **irá implementar parcialmente**.
> - Para cada item, **explique por que** não será implementado ou por
>   que será implementado parcialmente.

- Sombras dos carros: mapas de sombra em tempo real ou outras técnicas avançadas de sombreamento não serão implementadas por terem uma implementação mais complexa.
- Reflexos nos vidros dos carros: reflexos com mapas de cubo ou outras técnicas mais avançadas de reflexo também não serão implementadas por terem uma implementação mais complexa, a ideia é ao menos tentar refletir o skybox.
- Pista com bifurcações como em Mario Party: para o conceito proposto, consideramos melhor manter todos os carros percorrendo o mesmo trajeto. Os carros que partem nas faixas mais externas terão a distância que percorrem a mais compensada com um delocamento inicial ao invés de partirem lado a lado com a bifurcação compensando a diferença.
- Caso haja tempo hábil, as técnicas visuais citadas acima podem ser consideradas para implementação.

## Mecânica diferente do exemplo de Slot-Car Derby do Mario Party
Como citado acima, haverá uma mecânica que permite a troca de faixa pelos jogadores e que adiciona a necessidade de testes de intersecção. Essa mecânica visa adicionar mais estratégia para o formato da corrida, pois os jogadores que estiverem andando atrás de outros carros podem acumular uma certa quantidade de propulsão extra que por tempo limitado os permite andar mais rápido e sem sofrer penalidades por ultrapassar a velocidade máxima das curvas. Por outro lado, os jogadores que estiverem atrás podem ser penalizados por bater na traseira dos advesários se ainda não estiverem com a propulsão ativa.

> Comentário Professor: OK
