#include "c1.h"
#define cr102 &optab[0]

static char L1[]="b\301\n";
static char L2[]="GBb\243(I)\n";
#define cr100 &optab[3]

static char L3[]="bl\301\n";
static char L4[]="GBbl\243(I)\n";
static char L5[]="GAbl\250I)\n";
#define cr106 &optab[7]

static char L6[]="clr\311\n";
static char L7[]="clrf\311\n";
static char L8[]="li\311,A\n";
static char L9[]="movC\301,I\n>1\311,8\n";
static char L10[]="movof\301,I\n";
static char L11[]="GBmovC\243(I),I\n>1\311,8\n";
static char L12[]="GBmovof\243(I),I\n";
static char L13[]="li\311+,A+\nli\311,A\n";
static char L14[]="mov\301+,I+\nmov\301,I\n";
static char L15[]="GBmov\243+2(I),I+\nmov\243(I),I\n";
static char L16[]="GA";
#define cr32 &optab[28]

static char L17[]="mov\301',I\nM'\301''\n";
static char L18[]="mov\301',I\nM't\301''\n";
static char L19[]="mov\301',I\nli\3620,B\nM\3620,A''\n";
static char L20[]="movb\301',I\n>1\311,8\nli\3620,256*B\nMb\3620,A''\n";
static char L21[]="GJmov\243(J),I\nM'\243(J)\n";
static char L22[]="GBQmov\243(I),(sp)\nM'\243(I)\nmov\250sp)+,I\n";
static char L23[]="GJmov\243(J),I\nli\3620,B\nM\3620,#(J)\n";
static char L24[]="GJmovb\243(J),I\n>1\311,8\nli\3620,256*B\nMb\3620,#(J)\n";
static char L25[]="GBQmov\243(I),(sp)\nM\302,#(I)\nmov\250sp)+,I\n";
static char L26[]="GBQmovb\250I),(sp)\nli\3620,256*B\nMb\3620,(I)\nmovb\250sp)+,I\n>1\311,8\n";
static char L27[]="GAM'\301+\nV\301\n";
static char L28[]="GJmov\243+2(J),I+\nmov\243(J),I\nM'\243+2(J)\nV\243(J)\n";
static char L29[]="GBQmov\243+2(I),(sp)\nQmov\243(I),(sp)\nM'\243+2(I)\nV\243(I)\nmov\250sp)+,I\nmov\250sp)+,I+\n";
#define cr37 &optab[48]

static char L30[]="GAMP\311\n";
static char L31[]="GAM\311\nM\311+\nV\311\n";
#define cr80 &optab[53]

static char L32[]="KA<1\311,8\nmovC\311,A\n>1\311,8\n";
static char L33[]="KAmovfo\311,A\n";
static char L34[]="GBKAmovf\311,#(I)\n";
static char L35[]="GBli\3620,B*256\nmovb\3620,#(I)\n>1\3620,8\nmov\3620,I\n";
static char L36[]="GBli\3620,B\nmov\3620,#(I)\nmov\3620,I\n";
static char L37[]="GBmovb\302,#(I)\n";
static char L38[]="GBmov\302,r0\n<1\3620,8\nMC\3620,#(I)\n";
static char L39[]="GBmovC\302',#(I)\nmovC\302,I\n>1\311,8\n";
static char L40[]="GBKAmovfo\311,#(I)\n";
static char L41[]="GBKI<1\312,8\nmovC\312,#(I)\n>1\312,8\nmov\312,I\n";
static char L42[]="KAGJmovf\311,#(J)\n";
static char L43[]="KAGJmovfo\311,#(J)\n";
static char L44[]="GDKA<1\311,8\nmov\250sp)+,r0\nmovC\311,(r0)\n>1\311,8\n";
static char L45[]="GDKAmov\250sp)+,r0\nmovfo\311,(r0)\n";
static char L46[]="KAmov\311+,A+\nmov\311,A\n";
static char L47[]="KAGJmov\311+,#+2(J)\nmov\311,#(J)\n";
static char L48[]="GDKAmov\250sp)+,r0\nmov\311,(r0)+\nmov\311+,(r0)\n";
#define cr16 &optab[90]

static char L49[]="KAli\3620,256*Z\nszcC\3620,A'\nsla\311,8\nsocC\311,A''\n";
static char L50[]="KAli\3620,Z\nszcC\3620,A'\nsocC\311,A''\n";
#define L51 fas1

static char L52[]="KCGBli\3620,Z\nszcC\3620,#(I)\nsocC\250sp),#(I)\nmov\250sp)+,I\n";
#define cr45 &optab[96]

static char L53[]="GAM\311,B\n";
static char L54[]="GAmov\302,r0\n>2\3620,8\njeq\256+4\nM\311,0\n";
static char L55[]="GAKImov\312,r0\njeq\256+4\nM\311,0\n";
static char L56[]="KCGAmov\250sp)+,r0\njeq\256+4\nM\311,0\n";
static char L57[]="GA!li\3620,B\nbl\315'\n";
static char L58[]="GA!movD\302,r0\n>2\3620,8\njeq\256+6\nbl\315'\n";
static char L59[]="GA!KImov\312,r0\njeq\256+6\nbl\315'\n";
static char L60[]="KCGA!mov\250sp)+,r0\njeq\256+6\nbl\315'\n";
#define cr91 &optab[113]

static char L61[]="GAM\311\n";
static char L62[]="GAMt\311\n";
#define cr40 &optab[116]

static char L63[]="GA";
static char L64[]="li\311,A\nM\"\311,B\n";
static char L65[]="movC\301,I\n>1\311,8\nM\"\311,B\n";
static char L66[]="GAM\"\311,B\n";
#define add1 L67

static char L67[]="GAMD\302,I\n";
#define add2 L68

static char L68[]="GAKJMD\242(J),I\n";
#define add3 L69

static char L69[]="GAKIMP\312,I\n";
#define add5 L70

static char L70[]="KCGAMP\250sp)+,I\n";
static char L71[]="GAM\"\311+,B\nV\311\n";
static char L72[]="GAM\"\311,B\nM\"\311+,B+\nV\311\n";
static char L73[]="GAM\302,I+\nV\311\n";
static char L74[]="GAKIM\312,I+\nV\311\n";
static char L75[]="GAM\302,I\nM\302+,I+\nV\311\n";
#define addl1 L76

static char L76[]="GAKIM\312+,I+\nM\312,I\nV\311\n";
#define addl2 L77

static char L77[]="KCGAM\250sp)+,I\nM\250sp)+,I+\nV\311\n";
#define cr49 &optab[150]

#define L78 add3

static char L79[]="GCKAM\250sp)+,I\n";
#define L80 addl1

static char L81[]="KCGAM\250sp)+,I\nM\250sp)+,I+\n";
#define cr42 &optab[161]

static char L82[]="li\311,B\nmovb\301,r0\n>1\3620,8\nM\3620,I\n";
static char L83[]="li\311,B\nM\301,I\n";
static char L84[]="GAli\3620,B\nM\3620,I\n";
static char L85[]="GAMD\302,I\n";
static char L86[]="GAKJMD\242(J),I\n";
static char L87[]="GAKIMP\312,I\n";
static char L88[]="KCGAMP\250sp)+,I\n";
#define cr43 &optab[173]

static char L89[]="GA!KI!bl\315\n";
static char L90[]="KCGA!mov\250sp)+,J\nbl\315\n";
#define L91 add1

#define L92 add2

#define L93 add3

#define L94 add5

#define cr14 &optab[180]

static char L95[]="GAli\3620,B\ndiv\3620,I\n";
#define cr70 &optab[183]

static char L96[]="M'\301'\nxxx\nmov\301,I\n";
static char L97[]="M't\301'\nmov\301,I\n";
static char L98[]="movC\301',I\n>1\311,8\nM\"\311,B\n<1\311,8\nmovC\311,A\n>1\311,8\n";
#define addq1 L99

static char L99[]="M\302,A'\nmov\301,I\n";
#define addq20 L100

static char L100[]="movC\301',I\n>1\311,8\nM\311,B\n<1\311,8\nmovC\311,A\n>1\311,8\n";
#define addq1a L101

static char L101[]="movC\301',I\nMP\302,I\nmovC\311,A\n";
#define addq2 L102

static char L102[]="KBM\242(I),A'\nmov\301,I\n";
#define addq3 L103

static char L103[]="KAM\311,A'\nmov\301,I\n";
#define addq21 L104

static char L104[]="KCmovC\301',I\n>1\311,8\nM\250sp)+,I\n<1\311,8\nmovC\311,A\n>1\311,8\n";
#define addq4 L105

static char L105[]="KBGJM\242(I),#(J)\nmov\243(J),I\n";
#define addq4a L106

static char L106[]="movf\301',I\nKIMP\312,I\nmovf\311,A\n";
#define addq5 L107

static char L107[]="KCmovC\301',I\nMP\250sp)+,I\nmovC\311,A\n";
#define addq6 L108

static char L108[]="KCmovof\301',I\nMP\250sp)+,I\nmovfo\311,A''\n";
#define addq7 L109

static char L109[]="KAGJM\311,#(J)\nmov\243(J),I\n";
#define addq8 L110

static char L110[]="KCGBM\250sp)+,#(I)\nmov\243(I),I\n";
#define addq9 L111

static char L111[]="KCGBmov\311,r1\nmovC\243(r1),I\n>1\311,8\nMP\250sp)+,I\n<1\311,8\nmovC\311,#(r1)\n>1\311,8\n";
#define addq22 L112

#define L112 addq9

#define addq9a L113

static char L113[]="KCGBmovC\243(I),I\nMP\250sp)+,I\nmovC\311,#(I)\n";
#define addq10 L114

static char L114[]="KCGBmovof\243(I),J\nMP\250sp)+,J\nmovfo\312,#(I)\nmovf\312,I\n";
static char L115[]="M'\301+\nV\301\nGA";
static char L116[]="M't\301+\nV\301\nGA";
#define addq11 L117

static char L117[]="li\311,B\nM\311,A+\nV\301\nGA";
#define addq12 L118

static char L118[]="M\302+,A+\nV\301\nM\302,A\nGA";
#define addq13 L119

static char L119[]="KAM\311+,A+\nV\301\nM\311,A\nGA";
#define addq14 L120

static char L120[]="GBli\3620,B\nM\3620,#+2(I)\nV\243(I)\nmov\243+2(I),I+\nmov\243(I),I\n";
static char L121[]="GBli\3620,B+\nM\3620,#+2(I)\nV\243(I)\nli\3620,B\nM\3620,#(I)\nmov\243+2(I),I+\nmov\243(I),I\n";
#define addq15 L122

static char L122[]="GBM\302+,#+2(I)\nV\243(I)\nM\302,#(I)\nmov\243+2(I),I+\nmov\243(I),I\n";
#define addq16 L123

static char L123[]="KCGBM\250sp)+,#(I)\nM\250sp)+,#+2(I)\nV\243(I)\nmov\243+2(I),I+\nmov\243(I),I\n";
#define cr72 &optab[233]

static char L124[]="movC\301',I\n>1\311,8\nli\312,B\nM\312,I\n<1\312,8\nmovC\312,A\n";
static char L125[]="movC\301',I\n>1\311,8\nmovC\302,J\n>2\312,8\nM\312,I\n<1\312,8\nmovC\312,A\n";
static char L126[]="movC\301',I\n>1\311,8\nM\302,I\n<1\312,8\nmovC\312,A\n";
static char L127[]="KCmovC\301',I\n>1\311,8\nM\250sp)+,I\n<1\312,8\nmovC\312,A\n";
static char L128[]="GDKAmov\311,r0\nmov\250sp)+,r1\nmovC\243(r1),I\n>1\311,8\nM\3620,I\n<1\312,8\nmovC\312,#(r1)\n";
#define L129 addq1a

#define L130 addq4a

#define L131 addq5

#define L132 addq6

#define L133 addq9a

#define L134 addq10

#define cr73 &optab[252]

static char L135[]="movC\301',I\n>1\311,8\nKI!bl\315\n<1\311,8\nmovC\311,A\n>1\311,8\n";
static char L136[]="KCmovC\301',I\n>1\311,8\nmov\250sp)+,J\nbl\315\n<1\311,8\nmovC\311,A\n>1\311,8\n";
static char L137[]="GDKA!mov\311,J\nmov\250sp)+,r13\nmovC\243(r13),I\n>1\311,8\nbl\315\n<1\311,8\nmovC\311,#(r13)\n>1\311,8\n";
#define L138 addq1a

#define L139 addq4a

#define L140 addq5

#define L141 addq6

#define L142 addq9a

#define L143 addq10

#define cr79 &optab[265]

static char L144[]="KAM\301',I\nmov\311,A\n";
static char L145[]="KCmovb\301',I\n>1\311,8\nM\250sp)+,I\n<1\311,8\nmovb\311,A\n>1\311,8\n";
static char L146[]="GDKAmov\250sp)+,r1\nmovb\243(r1),r0\n>1\3620,8\nM\3620,I\n<1\311,8\nmovb\311,#(r1)\n>1\311,8\n";
static char L147[]="GDKAmov\250sp)+,r1\nM\250r1),I\nmov\311,(r1)\n";
#define cr75 &optab[272]

static char L148[]="M\301,B\nmov\301,I\n";
static char L149[]="KAmov\311,r0\njeq\256+4\nM\301,0\nmov\301,I\n";
static char L150[]="movC\301,I\n>1\311,8\nM\311,B\n<1\311,8\nmovC\311,A\n";
static char L151[]="KAmov\311,r0\nmovC\301,I\n>1\311,8\nM\311,0\n<1\311,8\nmovC\311,A\n";
static char L152[]="GA!li\3620,B\n>2\3620,8\njeq\256+6\nbl\315'\nmov\311+,A+\nmov\311,A\n";
static char L153[]="GA!KImov\312,r0\njeq\256+6\nbl\315'\nmov\311+,A+\nmov\311,A\n";
static char L154[]="KCGA!mov\250sp)+,r0\njeq\256+6\nbl\315'\nmov\311+,A+\nmov\311,A\n";
static char L155[]="KCGA!mov\250sp)+,r0\nmov\250sp)+,r0\njeq\256+6\nbl\315'\nmov\311+,A+\nmov\311,A\n";
#define cr78 &optab[287]

#define L156 addq1

static char L157[]="ML\302,A'\nclr\311\nbisb\301'',I\n";
#define L158 addq1a

#define L159 addq2

#define L160 addq3

static char L161[]="KCML\250sp)+,A'\nclr\311\nbisb\301'',I\n";
#define L162 addq4

#define L163 addq4a

#define L164 addq5

#define L165 addq6

#define L166 addq7

#define L167 addq8

#define L168 addq9

static char L169[]="GDKCML\250sp),*2(sp)\ntst\250sp)+\nclr\311\nbisb\252(sp)+,I\n";
#define L170 addq9a

#define L171 addq10

#define L172 addq11

#define L173 addq12

#define L174 addq13

#define L175 addq14

#define L176 addq15

#define L177 addq16

#define cr51 &optab[326]

static char L178[]="movif\301,I\n";
static char L179[]="GBmovif\243(I),I\n";
static char L180[]="GAmovif\311,I\n";
#define cr52 &optab[330]

static char L181[]="GAmovfi\311,I\n";
#define cr56 &optab[332]

static char L182[]="GAsetl\ndect\363p\ndect\363p\nmovfi\311,(sp)\nmov\250sp)+,I\nmov\250sp)+,I+\nseti\n";
#define cr57 &optab[334]

static char L183[]="setl\nmovif\301,I\nseti\n";
static char L184[]="GBsetl\nmovif\243(I),I\nseti\n";
static char L185[]="GCsetl\nmovif\250sp)+,I\nseti\n";
#define cr127 &optab[338]

static char L186[]="mov\301+,(sp)\ndect\363p\nmov\301,(sp)\nbl\315\nc\250sp)+,(sp)+\n";
static char L187[]="GBmov\243+2(I),(sp)\ndect\363p\nmov\243(I),(sp)\nbl\315\nc\250sp)+,(sp)+\n";
static char L188[]="GCbl\315\nc\250sp)+,(sp)+\n";
#define cr58 &optab[342]

static char L189[]="GI!clr\311\n";
static char L190[]="GAmov\311,J\nclr\311\n";
static char L191[]="GI!mov\312,I\nsra\311,15\n";
static char L192[]="GAmov\311,J\nsra\311,15\n";
#define cr59 &optab[347]

static char L193[]="mov\301+,I\n";
static char L194[]="GBmov\243+2(I),I\n";
#define cr82 &optab[352]

#define l82 L195

static char L195[]="KCGCbl\315\nai\363p,8\n";
#define cr121 &optab[357]

#define L196 l82

#define cr124 &optab[361]

#define L197 l86

#define cr86 &optab[366]

#define l86 L198

static char L198[]="KCGCbl\315\nai\363p,6\n";
#define cr109 &optab[369]

static char L199[]="GAsla\311,8\nsra\311,8\n";
#define cr117 &optab[371]

static char L200[]="GATli\3620,B\nM\3620,I-\n";
static char L201[]="GATM\302,I-\n";
static char L202[]="GATKJM\242(J),I-\n";
static char L203[]="GATKIM\312,I-\n";
static char L204[]="KCGATM\250sp)+,I-\n";
#define cr119 &optab[377]

static char L205[]="movC\301',I\n>1\311,8\nTli\3620,B\nM\3620,I-\n<1\311=,8\nmovC\311=,A\n>1\311=,8\n";
static char L206[]="movC\301',I\n>1\311,8\nTM\302,I-\n<1\311=,8\nmovC\311=,A\n>1\311=,8\n";
static char L207[]="KCmovC\301',I\n>1\311,8\nTM\250sp)+,I-\n<1\311=,8\nmovC\311=,A\n>1\311=,8\n";
static char L208[]="KCGJmovC\243(J),I\n>1\311,8\nTM\250sp)+,I-\n<1\311=,8\nmovC\311=,#(J)\n>1\311=,8\n";
static char L209[]="GDKCmov\3002(sp),r1\nmovC\243(r1),I\n>1\311,8\nTM\250sp)+,I-\n<1\311=,8\nmovC\311=,#(J)\n>1\311=,8\ninct\363p\n";
#define cr107 &optab[388]

static char L210[]="GA?sra\311,1\n";
#define cr130 &optab[390]

static char L211[]="GAli\312,B\ns\312,I\n";
static char L212[]="GAli\3620,B\ns\3620,I+\nV\311\n";
#define ci80 &optab[393]

static char L213[]="M'\301\n";
static char L214[]="M'\3620\nMC\3620,A\n";
static char L215[]="li\301,B\n";
static char L216[]="MD\302,A\n>2\301,8\n";
static char L217[]="KAM\311,A\n";
static char L218[]="li\311,256*B\nMC\311,A\n";
static char L219[]="MC\302,A\n";
static char L220[]="KA<1\311,8\nMC\311,A\n";
static char L221[]="M'C\301\n";
static char L222[]="GBclr\3620\nMC\3620,#(I)\n";
#define move2 L223

static char L223[]="GBM'C\243(I)\n";
static char L224[]="li\301,B\n";
static char L225[]="li\311,B\nM\311,A\n";
static char L226[]="li\311,B*256\nML\311,A\n";
#define move3 L227

static char L227[]="M\302,A\n";
#define move4 L228

static char L228[]="KBmovD\242(I),I\n>2\311,8\n<1\311,8\nMC\311,A\n";
#define move5 L229

static char L229[]="KA<1\311,8\nMC\311,A\n";
static char L230[]="GBli\3621,B*256\nMC\3621,#(I)\n";
static char L231[]="GBli\312,B\nMC\312,#(I)\n";
static char L232[]="GBmovb\302,#(I)\n";
#define move6 L233

static char L233[]="GBmov\302,r0\n<1\3620,8\nMC\3620,#(I)\n";
static char L234[]="GBM\302,#(I)\n";
#define move7 L235

static char L235[]="GBKJML\242(J),#(I)\n";
#define move8 L236

static char L236[]="GBKI<1\312,8\nMC\312,#(I)\n";
#define move9 L237

static char L237[]="KBGJML\242(I),#(J)\n";
#define move10 L238

static char L238[]="KAGJ<1\311,8\nMC\311,#(J)\n";
#define move11 L239

static char L239[]="GDKBmov\250sp)+,r1\nML\242(I),#(r1)\n";
#define move12 L240

static char L240[]="GDKAmov\250sp)+,r1\nMC\311,#(r1)\n";
static char L241[]="KAmovfi\311,A\n";
static char L242[]="KAGJmovfi\311,#(J)\n";
static char L243[]="clr\301\nclr\301+\n";
static char L244[]="GBclr\243(I)\nclr\243+2(I)\n";
static char L245[]="li\3620,B\nM\3620,A+\nsra\3620,15\nM\3620,A\n";
#define move13a L246

static char L246[]="M\302,A+\nV\301\n";
static char L247[]="KBmov\242(I),A+\nV\301\n";
static char L248[]="KAmov\311,A+\nV\301\n";
static char L249[]="KAsetl\nmovfi\311,A\nseti\n";
static char L250[]="KAGJsetl\nmovfi\311,#(J)\nseti\n";
static char L251[]="li\3620,B\nM\3620,A\nli\3620,B+\nM\3620,A+\n";
#define move13 L252

static char L252[]="M\302,A\nM\302+,A+\nV\301\n";
#define move14 L253

static char L253[]="KBM\242(I),A\nM\242+2(I),A+\nV\301\n";
#define move15 L254

static char L254[]="KAM\311,A\nM\311+,A+\nV\301\n";
#define move14a L255

static char L255[]="GBM\302,#+2(I)\nV\243(I)\n";
#define move16a L256

static char L256[]="GBM\302+,#+2(I)\nV\243(I)\nM\302,#(I)\n";
#define move16 L257

static char L257[]="KAGJM\311+,#+2(J)\nV\243(J)\nM\311,#(J)\n";
static char L258[]="KCGBmov\250sp)+,#+2(I)\nV\243(I)\n";
#define move17 L259

static char L259[]="KCGBM\250sp)+,#(I)\nM\250sp)+,#+2(I)\nV\243(I)\n";
#define ci78 &optab[498]

static char L260[]="M\"\301,B\n";
static char L261[]="GAM\"\311,B\n<1\311,8\nmovC\311,A\n";
#define L262 move3

static char L263[]="KAML\311,A\n";
#define L264 move5

static char L265[]="GBmovC\243(I),r0\nM\"\3620,B*256\nmovC\3620,#(I)\n";
static char L266[]="GBmov\243(I),r0\nM\"\3620,B\nmov\3620,#(I)\n";
#define L267 move6

#define L268 move7

#define L269 move8

#define L270 move9

#define L271 move10

#define L272 move11

#define L273 move12

#define L274 move13a

#define L275 move13

#define L276 move14

#define L277 move15

#define L278 move14a

#define L279 move16a

#define L280 move16

#define L281 move17

#define ci79 &optab[554]

static char L282[]="KAM\301,I\nM\301+,I+\nmov\311,A\nmov\311+,A+\n";
static char L283[]="KAGJM\243(J),I\nM\243+2(J),I+\nmov\311,#(J)\nmov\311+,#+2(J)\n";
static char L284[]="GDKAmov\250sp)+,r1\nM\243(r1),I\nM\243+2(r1),I+\nmov\311,#(J)\nmov\311+,#+2(J)\n";
#define ci70 &optab[567]

static char L285[]="M'\301\n";
static char L286[]="M't\301\n";
static char L287[]="M\"\301,B\n";
static char L288[]="li\3620,B*256\nMb\3620,A\n";
static char L289[]="li\3620,B\nM\3620,A\n";
#define L290 move3

#define L291 move4

#define L292 move5

static char L293[]="GBmov\243(I),r0\nM'\243(I)\nmov\3620,r0\n";
#define L294 move9

static char L295[]="KBmovC\301',J\n>1\312,8\nM\242(I),J\n<1\312,8\nmovC\312,A\n";
static char L296[]="KAmovC\301',J\n>1\312,8\nM\311,J\n<1\312,8\nmovC\312,A\n";
#define L297 move10

#define L298 move12

static char L299[]="KCGBmovC\243(I),J\n>1\312,8\nM\250sp)+,J\n<1\312,8\nmovC\312,#(I)\n";
static char L300[]="li\3620,B\nM\3620,A+\nV\301\nsra\3620,15\nM\3620,A\n";
static char L301[]="li\3620,B\nM\3620,A\nli\3620,B+\nM\3620,A+\nV\301\n";
#define L302 move13a

#define L303 move13

#define L304 move14

#define L305 move15

#define L306 move14a

#define L307 move16a

#define L308 move16

#define L309 move17

#define ci16 &optab[622]

static char L310[]="li\3620,Z*256\nszcb\3620,A'\nli\3620,B*256\nsocb\3620,A\n";
static char L311[]="li\3620,Z\nszc\3620,A'\nli\3620,B\nsoc\3620,A\n";
static char L312[]="li\3620,Z\nszcC\3620,A'\nli\3620,B\nsocC\3620,A\n";
static char L313[]="KAli\3620,Z\nszcC\3620,A'\n<1\311,8\nsocC\311,A\n";
static char L314[]="GBli\3620,Z\nszcC\3620,#(I)\nli\3620,B\nsocC\3620,#(I)\n";
#define fas1 L315

static char L315[]="KAGJli\3620,Z\nszcC\3620,#(J)\nsocC\311,#(J)\n";
static char L316[]="GBKIli\3620,Z\nszcC\3620,#(I)\nsocC\312,#(I)\n";
static char L317[]="KCGBli\3620,Z\nszcC\3620,#(I)\nsocC\250sp)+,#(I)\n";
#define cc60 &optab[631]

static char L318[]="li\3620,A\n";
static char L319[]="movC\301,r0\n";
static char L320[]="movof\301,I\n";
static char L321[]="GBmovC\243(I),r0\n";
static char L322[]="GBmovof\243(I),I\n";
static char L323[]="GE";
static char L324[]="ci\301,B\n";
static char L325[]="GAci\311,B\n";
static char L326[]="ML\301,B\n";
static char L327[]="GBML\243(I),B\n";
static char L328[]="GAMD\311,B\n";
static char L329[]="GBKJML\243(I),\"(J)\n";
static char L330[]="GBKIMC\243(I),J\n";
static char L331[]="GAKJMD\311,\"(J)\n";
static char L332[]="GAKIMP\311,J\n";
static char L333[]="KCGAMP\311,(sp)+\n";
static char L334[]="mov\301,r0\nX0mov\301+,r0\nX1";
static char L335[]="mov\301,r0\nX0mov\301+,r0\nci\3620,B\nX1";
static char L336[]="mov\301,r0\nci\3620,B\nX0mov\301+,r0\nci\3620,B+\nX1";
static char L337[]="li\3620,A\nM\3620,B\nX0li\3620,A+\nM\3620,B+\nX1";
static char L338[]="mov\301,r0\nX0M\301+,B\nX1";
#define lcmp1 L339

static char L339[]="M\301,B\nX0M\301+,B+\nX1";
static char L340[]="GBmov\243(I),r0\nX0mov\243+2(I),r0\nX1";
static char L341[]="GBmov\243(I),r0\nX0mov\243+2(I),r0\nci\3620,B\nX1";
static char L342[]="GBmov\243(I),r0\nX0M\243+2(I),B\nX1";
#define lcmp2 L343

static char L343[]="GBM\243(I),B\nX0M\243+2(I),B+\nX1";
static char L344[]="GAmov\311,r0\nX0mov\311+,r0\nX1";
static char L345[]="GAmov\311,r0\nX0ci\311+,B\nX1";
static char L346[]="GAci\311,B\nX0ci\311+,B+\nX1";
static char L347[]="GAmov\311,r0\nX0M\311+,B\nX1";
#define lcmp3 L348

static char L348[]="GAM\311,B\nX0M\311+,B+\nX1";
#define lcmp4 L349

static char L349[]="GBKJM\243(I),\"(J)\nX0M\243+2(I),\"+2(J)\nX1";
#define lcmp5 L350

static char L350[]="GAKJM\311,\"(J)\nX0M\311+,\"+2(J)\nX1";
#define lcmp6 L351

static char L351[]="GCKAQmov\311,(sp)\nmov\3004(sp),I\nmov\250sp)+,@2(sp)\nM\250sp)+,(sp)+\nX0M\311,I+\nX1";
#define cc81 &optab[711]

static char L352[]="movb\301,I\nM\"\311,B*256\n";
static char L353[]="GAM\"\311,B\n";
static char L354[]="KAmovC\301,r0\n>1\3620,8\nM\311,r0\n";
#define L355 move6

#define L356 add1

#define L357 add3

#define L358 add5

#define rest &optab[722]

static char L359[]="HA";
#define cs106 &optab[725]

static char L360[]="Qclr\250sp)\n";
static char L361[]="Qli\3620,A\nmov\3620,(sp)\n";
static char L362[]="Qmovb\301,r0\n>1\3620,8\nmov\3620,(sp)\n";
static char L363[]="Qmov\301,(sp)\n";
static char L364[]="GBQmov\243(I),(sp)\n";
static char L365[]="Qli\3620,A+\nmov\3620,(sp)\nQli\3620,A\nmov\3620,(sp)\n";
static char L366[]="Qmov\301+,(sp)\nQmov\301,(sp)\n";
#define cs91 &optab[736]

static char L367[]="GCM\250sp)\n";
static char L368[]="GCMt\250sp)\n";
#define cs40 &optab[739]

static char L369[]="QmovC\301,r0\n>1\3620,8\nM\"\3620,B\nmov\3620,(sp)\n";
static char L370[]="GCmov\250sp),r0\nM\"\3620,B\nmov\3620,(sp)\n";
static char L371[]="GCM\302,(sp)\n";
static char L372[]="GCKBM\242(I),(sp)\n";
static char L373[]="GCKAM\311,(sp)\n";
#define cs58 &optab[747]

static char L374[]="Qli\3620,A\nmov\3620,(sp)\nsra\3620,15\nQmov\3620,(sp)\n";
static char L375[]="GCQclr\250sp)\n";
static char L376[]="Qmov\301,r0\nmov\3620,(sp)\nsra\3620,15\nQmov\3620,(sp)\n";
#define cs56 &optab[751]

static char L377[]="GAsetl\nQmovfi\311,(sp)\nseti\n";
#define ci116 &optab[753]

static char L378[]="GA!KI!";
static char L379[]="KCGA!mov\250sp)+,r1\n";

/* goto (are these entries still used?) */
/* cr102 */

struct optab optab[]={
	{16,0,63,0,L1},	/* 0 */
	{127,0,63,0,L2},	/* 1 */
/* call */
	{0},
/* cr100 */
	{16,0,63,0,L3},	/* 3 */
	{127,0,63,0,L4},	/* 4 */
	{63,0,63,0,L5},	/* 5 */
/* load */
	{0},
/* cr106 */
	{4,0,63,0,L6},	/* 7 */
	{4,4,63,0,L7},	/* 8 */
	{8,0,63,0,L8},	/* 9 */
	{16,10,63,0,L9},	/* 10 */
	{16,3,63,0,L9},	/* 11 */
	{16,0,63,0,L9},	/* 12 */
	{16,5,63,0,L9},	/* 13 */
	{16,4,63,0,L10},	/* 14 */
	{127,10,63,0,L11},	/* 15 */
	{127,3,63,0,L11},	/* 16 */
	{127,0,63,0,L11},	/* 17 */
	{127,5,63,0,L11},	/* 18 */
	{127,4,63,0,L12},	/* 19 */
	{8,8,63,0,L13},	/* 20 */
	{8,11,63,0,L13},	/* 21 */
	{16,8,63,0,L14},	/* 22 */
	{16,11,63,0,L14},	/* 23 */
	{127,8,63,0,L15},	/* 24 */
	{127,11,63,0,L15},	/* 25 */
	{63,0,63,0,L16},	/* 26 */
/* ++,-- postfix; the right operand is always a CON */
	{0},
/* cr32 */
	{16,1,5,0,L17},	/* 28 */
	{16,1,6,0,L18},	/* 29 */
	{16,1,63,0,L19},	/* 30 */
	{16,3,63,0,L20},	/* 31 */
	{16,10,63,0,L20},	/* 32 */
	{84,1,5,0,L21},	/* 33 */
	{127,1,5,0,L22},	/* 34 */
	{84,1,63,0,L23},	/* 35 */
	{84,3,63,0,L24},	/* 36 */
	{84,10,63,0,L24},	/* 37 */
	{127,1,63,0,L25},	/* 38 */
	{127,3,63,0,L26},	/* 39 */
	{127,10,63,0,L26},	/* 40 */
	{16,8,5,0,L27},	/* 41 */
	{16,11,5,0,L27},	/* 42 */
	{84,8,5,0,L28},	/* 43 */
	{84,11,5,0,L28},	/* 44 */
	{127,8,5,0,L29},	/* 45 */
	{127,11,5,0,L29},	/* 46 */
/* - unary, ~ */
	{0},
/* cr37 */
	{63,0,63,0,L30},	/* 48 */
	{63,4,63,0,L30},	/* 49 */
	{63,8,63,0,L31},	/* 50 */
	{63,11,63,0,L31},	/* 51 */
/* = */
	{0},
/* cr80 */
	{16,10,63,0,L32},	/* 53 */
	{16,0,63,0,L32},	/* 54 */
	{16,5,63,4,L32},	/* 55 */
	{16,4,63,4,L33},	/* 56 */
	{127,5,16,4,L34},	/* 57 */
	{127,3,8,0,L35},	/* 58 */
	{127,10,8,0,L35},	/* 59 */
	{127,0,8,0,L36},	/* 60 */
	{127,3,16,3,L37},	/* 61 */
	{127,10,16,3,L37},	/* 62 */
	{127,3,16,10,L37},	/* 63 */
	{127,10,16,10,L37},	/* 64 */
	{127,3,16,0,L38},	/* 65 */
	{127,10,16,0,L38},	/* 66 */
	{127,0,16,1,L39},	/* 67 */
	{127,4,16,4,L40},	/* 68 */
	{127,10,20,0,L41},	/* 69 */
	{127,0,20,0,L41},	/* 70 */
	{84,5,63,4,L42},	/* 71 */
	{84,4,63,4,L43},	/* 72 */
	{127,10,63,0,L44},	/* 73 */
	{127,0,63,0,L44},	/* 74 */
	{127,5,63,4,L44},	/* 75 */
	{127,4,63,4,L45},	/* 76 */
	{16,8,63,8,L46},	/* 77 */
	{16,8,63,11,L46},	/* 78 */
	{16,11,63,8,L46},	/* 79 */
	{16,11,63,11,L46},	/* 80 */
	{84,8,63,8,L47},	/* 81 */
	{84,8,63,11,L47},	/* 82 */
	{84,11,63,8,L47},	/* 83 */
	{84,11,63,11,L47},	/* 84 */
	{127,8,63,8,L48},	/* 85 */
	{127,8,63,11,L48},	/* 86 */
	{127,11,63,8,L48},	/* 87 */
	{127,11,63,11,L48},	/* 88 */
/* field assign, value in reg. */
	{0},
/* cr16 */
	{16,3,63,0,L49},	/* 90 */
	{16,10,63,0,L49},	/* 91 */
	{16,0,63,0,L50},	/* 92 */
	{84,0,63,0,L51},	/* 93 */


	{127,0,63,0,L52},	/* 94 */
/* <<, >>, unsigned >> */
	{0},
/* cr45 */
	{63,10,8,0,L53},	/* 96 */
	{63,0,8,0,L53},	/* 97 */
	{63,10,16,0,L54},	/* 98 */
	{63,0,16,0,L54},	/* 99 */
	{63,10,20,0,L55},	/* 100 */
	{63,0,20,0,L55},	/* 101 */
	{63,10,63,0,L56},	/* 102 */
	{63,0,63,0,L56},	/* 103 */
	{63,8,8,0,L57},	/* 104 */
	{63,11,8,0,L57},	/* 105 */
	{63,11,16,0,L58},	/* 106 */
	{63,8,16,0,L58},	/* 107 */
	{63,11,20,0,L59},	/* 108 */
	{63,8,20,0,L59},	/* 109 */
	{63,11,63,0,L60},	/* 110 */
	{63,8,63,0,L60},	/* 111 */
/* +1, +2, -1, -2 */
	{0},
/* cr91 */
	{63,0,5,0,L61},	/* 113 */
	{63,0,6,0,L62},	/* 114 */
/* +, -, |, &~ */
	{0},
/* cr40 */
	{63,0,4,0,L63},	/* 116 */
	{8,0,8,0,L64},	/* 117 */
	{16,10,8,0,L65},	/* 118 */
	{16,0,8,0,L65},	/* 119 */
	{63,0,8,0,L66},	/* 120 */
	{63,0,16,1,L67},	/* 121 */
	{63,4,16,5,L67},	/* 122 */
	{63,0,84,1,L68},	/* 123 */
	{63,4,84,5,L68},	/* 124 */
	{63,0,20,0,L69},	/* 125 */
	{63,4,20,4,L69},	/* 126 */
	{63,0,63,0,L70},	/* 127 */
	{63,4,63,4,L70},	/* 128 */
	{63,8,8,0,L71},	/* 129 */
	{63,11,8,0,L71},	/* 130 */
	{63,8,8,8,L72},	/* 131 */
	{63,11,8,8,L72},	/* 132 */
	{63,8,16,0,L73},	/* 133 */
	{63,11,16,0,L73},	/* 134 */
	{63,8,20,0,L74},	/* 135 */
	{63,11,20,0,L74},	/* 136 */
	{63,8,16,8,L75},	/* 137 */
	{63,8,16,11,L75},	/* 138 */
	{63,11,16,8,L75},	/* 139 */
	{63,11,16,11,L75},	/* 140 */
	{63,8,20,8,L76},	/* 141 */
	{63,8,20,11,L76},	/* 142 */
	{63,11,20,8,L76},	/* 143 */
	{63,11,20,11,L76},	/* 144 */
	{63,8,63,8,L77},	/* 145 */
	{63,8,63,11,L77},	/* 146 */
	{63,11,63,8,L77},	/* 147 */
	{63,11,63,11,L77},	/* 148 */
/* ^ -- xor */
	{0},
/* cr49 */
	{63,0,20,0,L78},	/* 150 */


	{63,0,63,0,L79},	/* 151 */
	{63,8,20,8,L80},	/* 152 */
	{63,8,20,11,L80},	/* 153 */
	{63,11,20,8,L80},	/* 154 */
	{63,11,20,11,L80},	/* 155 */


	{63,8,63,8,L81},	/* 156 */
	{63,8,63,11,L81},	/* 157 */
	{63,11,63,8,L81},	/* 158 */
	{63,11,63,11,L81},	/* 159 */
/* '*' -- low word of result is okay for both signed and unsigned.
 * R is increased by 1 following these snippets.
 */
	{0},
/* cr42 */
	{16,3,8,0,L82},	/* 161 */
	{16,0,8,0,L83},	/* 162 */
	{63,0,8,0,L84},	/* 163 */
	{63,0,16,1,L85},	/* 164 */
	{63,4,16,5,L85},	/* 165 */
	{63,0,84,1,L86},	/* 166 */
	{63,4,84,5,L86},	/* 167 */
	{63,0,20,0,L87},	/* 168 */
	{63,4,20,4,L87},	/* 169 */
	{63,0,63,0,L88},	/* 170 */
	{63,4,63,4,L88},	/* 171 */
/* / and % -- signed */
	{0},
/* cr43 */
	{63,0,20,0,L89},	/* 173 */
	{63,0,63,0,L90},	/* 174 */
	{63,4,16,5,L91},	/* 175 */


	{63,4,84,5,L92},	/* 176 */


	{63,4,20,4,L93},	/* 177 */


	{63,4,63,4,L94},	/* 178 */


/* PTOI */
	{0},
/* cr14 */
	{63,8,16,0,L95},	/* 180 */
	{63,11,16,0,L95},	/* 181 */
/* +=, -= */
	{0},
/* cr70 */
	{16,1,5,0,L96},	/* 183 */
	{16,1,6,0,L97},	/* 184 */
	{16,10,8,0,L98},	/* 185 */
	{16,0,8,0,L98},	/* 186 */
	{16,1,16,1,L99},	/* 187 */
	{16,10,16,1,L100},	/* 188 */
	{16,0,16,1,L100},	/* 189 */
	{16,5,16,5,L101},	/* 190 */
	{16,1,127,1,L102},	/* 191 */
	{16,1,63,0,L103},	/* 192 */
	{16,0,63,0,L104},	/* 193 */
	{16,10,63,0,L104},	/* 194 */
	{84,1,127,1,L105},	/* 195 */
	{16,5,20,4,L106},	/* 196 */
	{16,5,63,4,L107},	/* 197 */
	{16,4,63,4,L108},	/* 198 */
	{84,1,63,0,L109},	/* 199 */
	{127,1,63,0,L110},	/* 200 */
	{127,10,63,0,L111},	/* 201 */
	{127,0,63,0,L111},	/* 202 */
	{127,10,63,0,L112},	/* 203 */


	{127,5,63,4,L113},	/* 204 */
	{127,4,63,4,L114},	/* 205 */
	{16,8,5,0,L115},	/* 206 */
	{16,11,5,0,L115},	/* 207 */
	{16,8,6,0,L116},	/* 208 */
	{16,11,6,0,L116},	/* 209 */
	{16,8,8,0,L117},	/* 210 */
	{16,11,8,0,L117},	/* 211 */
	{16,8,16,8,L118},	/* 212 */
	{16,8,16,11,L118},	/* 213 */
	{16,11,16,8,L118},	/* 214 */
	{16,11,16,11,L118},	/* 215 */
	{16,8,63,8,L119},	/* 216 */
	{16,8,63,11,L119},	/* 217 */
	{16,11,63,8,L119},	/* 218 */
	{16,11,63,11,L119},	/* 219 */
	{127,8,8,0,L120},	/* 220 */
	{127,11,8,0,L120},	/* 221 */
	{127,8,8,8,L121},	/* 222 */
	{127,11,8,8,L121},	/* 223 */
	{127,8,16,8,L122},	/* 224 */
	{127,8,16,11,L122},	/* 225 */
	{127,11,16,8,L122},	/* 226 */
	{127,11,16,11,L122},	/* 227 */
	{127,8,63,8,L123},	/* 228 */
	{127,8,63,11,L123},	/* 229 */
	{127,11,63,8,L123},	/* 230 */
	{127,11,63,11,L123},	/* 231 */
/* '*=' */
	{0},
/* cr72 */
	{16,10,8,0,L124},	/* 233 */
	{16,0,8,0,L124},	/* 234 */
	{16,10,16,3,L125},	/* 235 */
	{16,10,16,10,L125},	/* 236 */
	{16,0,16,3,L125},	/* 237 */
	{16,0,16,10,L125},	/* 238 */
	{16,10,16,1,L126},	/* 239 */
	{16,0,16,1,L126},	/* 240 */
	{16,10,63,0,L127},	/* 241 */
	{16,0,63,0,L127},	/* 242 */
	{127,10,63,0,L128},	/* 243 */
	{127,0,63,0,L128},	/* 244 */
	{16,5,16,5,L129},	/* 245 */


	{16,5,20,4,L130},	/* 246 */


	{16,5,63,4,L131},	/* 247 */


	{16,4,63,4,L132},	/* 248 */


	{127,5,63,4,L133},	/* 249 */


	{127,4,63,4,L134},	/* 250 */


/* /= and %= -- signed int */
	{0},
/* cr73 */
	{16,0,20,0,L135},	/* 252 */
	{16,10,20,0,L135},	/* 253 */
	{16,0,63,0,L136},	/* 254 */
	{16,10,63,0,L136},	/* 255 */
	{127,0,63,0,L137},	/* 256 */
	{127,10,63,0,L137},	/* 257 */
	{16,5,16,5,L138},	/* 258 */


	{16,5,20,4,L139},	/* 259 */


	{16,5,63,4,L140},	/* 260 */


	{16,4,63,4,L141},	/* 261 */


	{127,5,63,4,L142},	/* 262 */


	{127,4,63,4,L143},	/* 263 */


/* ^= -- =xor */
	{0},
/* cr79 */
	{16,1,63,0,L144},	/* 265 */
	{16,10,63,0,L145},	/* 266 */
	{16,3,63,0,L145},	/* 267 */
	{127,10,63,0,L146},	/* 268 */
	{127,3,63,0,L146},	/* 269 */
	{127,0,63,0,L147},	/* 270 */
/* <<=, >>=, unsigned >>= */
	{0},
/* cr75 */
	{9,0,8,0,L148},	/* 272 */
	{9,0,63,0,L149},	/* 273 */
	{16,10,8,0,L150},	/* 274 */
	{16,0,8,0,L150},	/* 275 */
	{16,10,63,0,L151},	/* 276 */
	{16,0,63,0,L151},	/* 277 */
	{16,11,8,0,L152},	/* 278 */
	{16,8,8,0,L152},	/* 279 */
	{16,11,20,0,L153},	/* 280 */
	{16,8,20,0,L153},	/* 281 */
	{16,11,63,0,L154},	/* 282 */
	{16,8,63,0,L154},	/* 283 */
/* with a long shift ignore the high word */
	{16,11,63,8,L155},	/* 284 */
	{16,8,63,8,L155},	/* 285 */
/* =|, =&~ */
	{0},
/* cr78 */
	{16,1,16,1,L156},	/* 287 */


	{16,10,16,0,L157},	/* 288 */
	{16,0,16,1,L158},	/* 289 */
	{16,5,16,5,L158},	/* 290 */


	{16,1,127,1,L159},	/* 291 */


	{16,1,63,0,L160},	/* 292 */


	{16,10,63,0,L161},	/* 293 */
	{84,1,127,1,L162},	/* 294 */


	{16,5,20,4,L163},	/* 295 */


	{16,0,63,0,L164},	/* 296 */
	{16,5,63,4,L164},	/* 297 */


	{16,4,63,4,L165},	/* 298 */


	{84,1,63,0,L166},	/* 299 */


	{127,1,63,0,L167},	/* 300 */


	{127,0,63,0,L168},	/* 301 */


	{127,10,63,0,L169},	/* 302 */
	{127,5,63,4,L170},	/* 303 */


	{127,4,63,4,L171},	/* 304 */


	{16,8,8,0,L172},	/* 305 */
	{16,11,8,0,L172},	/* 306 */


	{16,8,16,8,L173},	/* 307 */
	{16,8,16,11,L173},	/* 308 */
	{16,11,16,8,L173},	/* 309 */
	{16,11,16,11,L173},	/* 310 */


	{16,8,63,8,L174},	/* 311 */
	{16,8,63,11,L174},	/* 312 */
	{16,11,63,8,L174},	/* 313 */
	{16,11,63,11,L174},	/* 314 */


	{127,8,8,0,L175},	/* 315 */
	{127,11,8,0,L175},	/* 316 */


	{127,8,16,8,L176},	/* 317 */
	{127,8,16,11,L176},	/* 318 */
	{127,11,16,8,L176},	/* 319 */
	{127,11,16,11,L176},	/* 320 */


	{127,8,63,8,L177},	/* 321 */
	{127,8,63,11,L177},	/* 322 */
	{127,11,63,8,L177},	/* 323 */
	{127,11,63,11,L177},	/* 324 */


/* int -> float */
	{0},
/* cr51 */
	{16,1,63,0,L178},	/* 326 */
	{127,1,63,0,L179},	/* 327 */
	{63,0,63,0,L180},	/* 328 */
/* float, double -> int */
	{0},
/* cr52 */
	{63,4,63,0,L181},	/* 330 */
/* double (float) to long */
	{0},
/* cr56 */
	{63,4,63,0,L182},	/* 332 */
/* long to double */
	{0},
/* cr57 */
	{16,8,63,0,L183},	/* 334 */
	{127,8,63,0,L184},	/* 335 */
	{63,8,63,0,L185},	/* 336 */
/* unsigned long to float(double) */
	{0},
/* cr127 */
	{16,11,63,0,L186},	/* 338 */
	{127,11,63,0,L187},	/* 339 */
	{63,11,63,0,L188},	/* 340 */
/* integer to long */
	{0},
/* cr58 */
	{20,9,63,0,L189},	/* 342 */
	{63,9,63,0,L190},	/* 343 */
	{20,0,63,0,L191},	/* 344 */
	{63,0,63,0,L192},	/* 345 */
/* long to integer */
	{0},
/* cr59 */
	{16,8,63,0,L193},	/* 347 */
	{16,11,63,0,L193},	/* 348 */
	{127,8,63,0,L194},	/* 349 */
	{127,11,63,0,L194},	/* 350 */
/* *, /, % for longs. */
	{0},
/* cr82 */
	{63,8,63,8,L195},	/* 352 */
	{63,8,63,11,L195},	/* 353 */
	{63,11,63,8,L195},	/* 354 */
	{63,11,63,11,L195},	/* 355 */
/* *, /, % for unsigned long */
	{0},
/* cr121 */
	{63,11,63,8,L196},	/* 357 */
	{63,8,63,11,L196},	/* 358 */
	{63,11,63,11,L196},	/* 359 */


/* *=, /=, %= for unsigned long */
	{0},
/* cr124 */
	{63,0,63,8,L197},	/* 361 */
	{63,0,63,11,L197},	/* 362 */
	{63,8,63,0,L197},	/* 363 */
	{63,11,63,0,L197},	/* 364 */


/* *=, /=, %= for longs */
/* Operands of the form &x op y, so stack space is known. */
	{0},
/* cr86 */
	{63,0,63,8,L198},	/* 366 */
	{63,0,63,11,L198},	/* 367 */
/* convert integer to character (sign extend) */
	{0},
/* cr109 */
	{63,0,63,0,L199},	/* 369 */
/* / and % where divisor is unsigned or known to be positive */
	{0},
/* cr117 */
	{63,0,8,0,L200},	/* 371 */
	{63,0,16,1,L201},	/* 372 */
	{63,0,84,1,L202},	/* 373 */
	{63,0,20,0,L203},	/* 374 */
	{63,0,63,0,L204},	/* 375 */

/* /= and %= where divisor is unsigned or known to be positive */
	{0},
/* cr119 */
	{16,10,8,0,L205},	/* 377 */
	{16,0,8,0,L205},	/* 378 */
	{16,10,16,1,L206},	/* 379 */
	{16,0,16,1,L206},	/* 380 */
	{16,10,63,0,L207},	/* 381 */
	{16,0,63,0,L207},	/* 382 */
	{84,10,63,0,L208},	/* 383 */
	{84,0,63,0,L208},	/* 384 */
	{127,10,63,0,L209},	/* 385 */
	{127,0,63,0,L209},	/* 386 */
/* (int *) - (int *) */
	{0},
/* cr107 */
	{63,0,63,0,L210},	/* 388 */
/* x - &name */
	{0},
/* cr130 */
	{63,0,63,0,L211},	/* 390 */
	{63,8,63,0,L212},	/* 391 */


/* = */
	{0},
/* ci80 */
	{9,0,4,0,L213},	/* 393 */
	{16,3,4,0,L214},	/* 394 */
	{16,10,4,0,L214},	/* 395 */
	{9,0,8,0,L215},	/* 396 */
	{9,0,16,0,L216},	/* 397 */
	{9,0,16,10,L216},	/* 398 */
	{9,0,63,0,L217},	/* 399 */
	{16,3,8,0,L218},	/* 400 */
	{16,10,8,0,L218},	/* 401 */
	{16,3,16,3,L219},	/* 402 */
	{16,3,16,10,L219},	/* 403 */
	{16,10,16,3,L219},	/* 404 */
	{16,10,16,10,L219},	/* 405 */
	{16,3,63,0,L220},	/* 406 */
	{16,10,63,0,L220},	/* 407 */
	{16,1,4,0,L221},	/* 408 */
	{16,5,4,4,L221},	/* 409 */
	{127,3,4,0,L222},	/* 410 */
	{127,10,4,0,L222},	/* 411 */
	{127,0,4,0,L223},	/* 412 */
	{127,5,4,4,L223},	/* 413 */
	{127,10,4,0,L223},	/* 414 */
	{9,0,8,0,L224},	/* 415 */
	{16,0,8,0,L225},	/* 416 */
	{16,3,8,0,L226},	/* 417 */
	{16,10,8,0,L226},	/* 418 */
	{16,1,16,1,L227},	/* 419 */
	{16,0,127,0,L228},	/* 420 */
	{16,10,127,0,L228},	/* 421 */
	{16,0,63,0,L229},	/* 422 */
	{16,10,63,0,L229},	/* 423 */
	{127,3,8,0,L230},	/* 424 */
	{127,10,8,0,L230},	/* 425 */
	{127,0,8,0,L231},	/* 426 */
	{127,3,16,3,L232},	/* 427 */
	{127,10,16,3,L232},	/* 428 */
	{127,3,16,10,L232},	/* 429 */
	{127,10,16,10,L232},	/* 430 */
	{127,3,16,0,L233},	/* 431 */
	{127,10,16,0,L233},	/* 432 */
	{127,0,16,0,L234},	/* 433 */
	{127,0,84,1,L235},	/* 434 */
	{127,3,84,0,L235},	/* 435 */
	{127,10,84,0,L235},	/* 436 */
	{127,10,20,0,L236},	/* 437 */
	{127,0,20,0,L236},	/* 438 */
	{84,0,127,1,L237},	/* 439 */
	{84,3,127,0,L237},	/* 440 */
	{84,10,127,0,L237},	/* 441 */
	{84,0,63,0,L238},	/* 442 */
	{84,10,63,0,L238},	/* 443 */
	{127,0,127,1,L239},	/* 444 */
	{127,3,127,0,L239},	/* 445 */
	{127,10,127,0,L239},	/* 446 */
	{127,0,63,0,L240},	/* 447 */
	{127,10,63,0,L240},	/* 448 */
	{16,1,63,4,L241},	/* 449 */
	{84,1,63,4,L242},	/* 450 */
	{16,8,4,0,L243},	/* 451 */
	{16,11,4,0,L243},	/* 452 */
	{127,8,4,0,L244},	/* 453 */
	{127,11,4,0,L244},	/* 454 */

	{16,8,8,1,L245},	/* 455 */
	{16,11,8,1,L245},	/* 456 */
	{16,8,16,1,L246},	/* 457 */
	{16,11,16,1,L246},	/* 458 */
	{16,8,127,1,L247},	/* 459 */
	{16,11,127,1,L247},	/* 460 */
	{16,8,63,0,L248},	/* 461 */
	{16,11,63,0,L248},	/* 462 */
	{16,8,63,4,L249},	/* 463 */
	{16,11,63,4,L249},	/* 464 */
	{84,8,63,4,L250},	/* 465 */
	{84,11,63,4,L250},	/* 466 */
	{16,8,8,8,L251},	/* 467 */
	{16,11,8,8,L251},	/* 468 */
	{16,8,16,8,L252},	/* 469 */
	{16,8,16,11,L252},	/* 470 */
	{16,11,16,8,L252},	/* 471 */
	{16,11,16,11,L252},	/* 472 */
	{16,8,127,8,L253},	/* 473 */
	{16,8,127,11,L253},	/* 474 */
	{16,11,127,8,L253},	/* 475 */
	{16,11,127,11,L253},	/* 476 */
	{16,8,63,8,L254},	/* 477 */
	{16,8,63,11,L254},	/* 478 */
	{16,11,63,8,L254},	/* 479 */
	{16,11,63,11,L254},	/* 480 */
	{127,8,16,1,L255},	/* 481 */
	{127,11,16,1,L255},	/* 482 */
	{127,8,16,8,L256},	/* 483 */
	{127,8,16,11,L256},	/* 484 */
	{127,11,16,8,L256},	/* 485 */
	{127,11,16,11,L256},	/* 486 */
	{84,8,63,8,L257},	/* 487 */
	{84,8,63,11,L257},	/* 488 */
	{84,11,63,8,L257},	/* 489 */
	{84,11,63,11,L257},	/* 490 */
	{127,8,63,0,L258},	/* 491 */
	{127,11,63,0,L258},	/* 492 */
	{127,8,63,8,L259},	/* 493 */
	{127,8,63,11,L259},	/* 494 */
	{127,11,63,8,L259},	/* 495 */
	{127,11,63,11,L259},	/* 496 */
/* |= and &~= */
	{0},
/* ci78 */
	{9,0,8,0,L260},	/* 498 */
	{16,0,8,0,L261},	/* 499 */
	{16,10,8,0,L261},	/* 500 */
	{16,1,16,1,L262},	/* 501 */


	{16,10,63,0,L263},	/* 502 */
	{16,0,63,0,L264},	/* 503 */
	{16,10,63,0,L264},	/* 504 */


	{127,3,8,0,L265},	/* 505 */
	{127,10,8,0,L265},	/* 506 */
	{127,0,8,0,L266},	/* 507 */
	{127,0,16,1,L267},	/* 508 */
	{127,3,16,0,L267},	/* 509 */
	{127,10,16,0,L267},	/* 510 */


	{127,0,84,1,L268},	/* 511 */
	{127,3,84,0,L268},	/* 512 */
	{127,10,84,0,L268},	/* 513 */


	{127,0,20,0,L269},	/* 514 */


	{84,0,127,1,L270},	/* 515 */
	{84,3,127,0,L270},	/* 516 */
	{84,10,127,0,L270},	/* 517 */


	{84,0,63,0,L271},	/* 518 */


	{127,0,127,1,L272},	/* 519 */
	{127,3,127,0,L272},	/* 520 */
	{127,10,127,0,L272},	/* 521 */


	{127,0,63,0,L273},	/* 522 */


	{16,8,8,0,L274},	/* 523 */
	{16,8,16,9,L274},	/* 524 */
	{16,11,8,0,L274},	/* 525 */
	{16,11,16,9,L274},	/* 526 */


	{16,8,16,8,L275},	/* 527 */
	{16,8,16,11,L275},	/* 528 */
	{16,11,16,8,L275},	/* 529 */
	{16,11,16,11,L275},	/* 530 */


	{16,8,127,8,L276},	/* 531 */
	{16,8,127,11,L276},	/* 532 */
	{16,11,127,8,L276},	/* 533 */
	{16,11,127,11,L276},	/* 534 */


	{16,8,63,8,L277},	/* 535 */
	{16,8,63,11,L277},	/* 536 */
	{16,11,63,8,L277},	/* 537 */
	{16,11,63,11,L277},	/* 538 */


	{127,8,8,0,L278},	/* 539 */
	{127,11,8,0,L278},	/* 540 */


	{127,8,16,8,L279},	/* 541 */
	{127,8,16,11,L279},	/* 542 */
	{127,11,16,8,L279},	/* 543 */
	{127,11,16,11,L279},	/* 544 */


	{84,8,63,8,L280},	/* 545 */
	{84,8,63,11,L280},	/* 546 */
	{84,11,63,8,L280},	/* 547 */
	{84,11,63,11,L280},	/* 548 */


	{127,8,63,8,L281},	/* 549 */
	{127,8,63,11,L281},	/* 550 */
	{127,11,63,8,L281},	/* 551 */
	{127,11,63,11,L281},	/* 552 */


/* ^= */
	{0},
/* ci79 */
	{16,8,63,8,L282},	/* 554 */
	{16,8,63,11,L282},	/* 555 */
	{16,11,63,8,L282},	/* 556 */
	{16,11,63,11,L282},	/* 557 */
	{84,8,63,8,L283},	/* 558 */
	{84,8,63,11,L283},	/* 559 */
	{84,11,63,8,L283},	/* 560 */
	{84,11,63,11,L283},	/* 561 */
	{127,8,63,8,L284},	/* 562 */
	{127,8,63,11,L284},	/* 563 */
	{127,11,63,8,L284},	/* 564 */
	{127,11,63,11,L284},	/* 565 */
/* +=, -=, ++, -- */
	{0},
/* ci70 */
	{16,1,5,0,L285},	/* 567 */
	{16,1,6,0,L286},	/* 568 */
	{9,0,8,0,L287},	/* 569 */
	{16,3,8,0,L288},	/* 570 */
	{16,10,8,0,L288},	/* 571 */
	{16,0,8,0,L289},	/* 572 */
	{16,1,16,1,L290},	/* 573 */


	{16,0,127,0,L291},	/* 574 */
	{16,10,127,0,L291},	/* 575 */


	{16,0,63,0,L292},	/* 576 */
	{16,10,63,0,L292},	/* 577 */


	{127,1,5,0,L293},	/* 578 */
	{84,1,127,1,L294},	/* 579 */


	{16,0,84,1,L295},	/* 580 */
	{16,10,84,1,L295},	/* 581 */
	{16,0,63,0,L296},	/* 582 */
	{16,10,63,0,L296},	/* 583 */
	{84,1,63,0,L297},	/* 584 */


	{127,1,63,0,L298},	/* 585 */


	{127,0,63,0,L299},	/* 586 */
	{16,8,8,0,L300},	/* 587 */
	{16,11,8,0,L300},	/* 588 */
	{16,8,8,8,L301},	/* 589 */
	{16,11,8,8,L301},	/* 590 */
	{16,8,16,9,L302},	/* 591 */
	{16,11,16,9,L302},	/* 592 */


	{16,8,16,8,L303},	/* 593 */
	{16,8,16,11,L303},	/* 594 */
	{16,11,16,8,L303},	/* 595 */
	{16,11,16,11,L303},	/* 596 */


	{16,8,127,8,L304},	/* 597 */
	{16,8,127,11,L304},	/* 598 */
	{16,11,127,8,L304},	/* 599 */
	{16,11,127,11,L304},	/* 600 */


	{16,8,63,8,L305},	/* 601 */
	{16,8,63,11,L305},	/* 602 */
	{16,11,63,8,L305},	/* 603 */
	{16,11,63,11,L305},	/* 604 */


	{127,8,8,0,L306},	/* 605 */
	{127,8,16,9,L306},	/* 606 */
	{127,11,8,0,L306},	/* 607 */
	{127,11,16,9,L306},	/* 608 */


	{127,8,16,8,L307},	/* 609 */
	{127,8,16,11,L307},	/* 610 */
	{127,11,16,8,L307},	/* 611 */
	{127,11,16,11,L307},	/* 612 */


	{84,8,63,8,L308},	/* 613 */
	{84,8,63,11,L308},	/* 614 */
	{84,11,63,8,L308},	/* 615 */
	{84,11,63,11,L308},	/* 616 */


	{127,8,63,8,L309},	/* 617 */
	{127,8,63,11,L309},	/* 618 */
	{127,11,63,8,L309},	/* 619 */
	{127,11,63,11,L309},	/* 620 */


/* field = ... */
	{0},
/* ci16 */
	{16,3,8,0,L310},	/* 622 */
	{16,0,8,0,L311},	/* 623 */
	{16,0,16,0,L312},	/* 624 */
	{16,0,63,0,L313},	/* 625 */
	{127,0,16,0,L314},	/* 626 */
	{84,0,63,0,L315},	/* 627 */
	{127,0,20,0,L316},	/* 628 */
	{127,0,63,0,L317},	/* 629 */


/* relationals */
	{0},
/* cc60 */
	{8,0,4,0,L318},	/* 631 */
	{16,0,4,0,L319},	/* 632 */
	{16,5,4,4,L319},	/* 633 */
	{16,10,4,0,L319},	/* 634 */
	{16,4,4,0,L320},	/* 635 */
	{127,0,4,0,L321},	/* 636 */
	{127,5,4,4,L321},	/* 637 */
	{127,10,4,0,L321},	/* 638 */
	{127,4,4,0,L322},	/* 639 */
	{63,0,4,0,L323},	/* 640 */
	{63,4,4,4,L323},	/* 641 */
	{9,0,8,0,L324},	/* 642 */
	{63,0,8,0,L325},	/* 643 */
	{16,1,16,1,L326},	/* 644 */
	{16,3,16,3,L326},	/* 645 */
	{16,10,16,10,L326},	/* 646 */
	{127,1,16,1,L327},	/* 647 */
	{127,3,16,3,L327},	/* 648 */
	{127,10,16,10,L327},	/* 649 */
	{63,0,16,1,L328},	/* 650 */
	{63,4,16,5,L328},	/* 651 */
	{127,1,84,1,L329},	/* 652 */
	{127,3,84,3,L329},	/* 653 */
	{127,10,84,10,L329},	/* 654 */
	{127,1,20,0,L330},	/* 655 */
	{63,0,84,1,L331},	/* 656 */
	{63,4,84,5,L331},	/* 657 */
	{63,0,20,0,L332},	/* 658 */
	{63,4,20,4,L332},	/* 659 */
	{63,0,63,0,L333},	/* 660 */
	{63,4,63,4,L333},	/* 661 */
	{16,8,4,0,L334},	/* 662 */
	{16,11,4,0,L334},	/* 663 */
	{16,8,8,0,L335},	/* 664 */
	{16,11,8,0,L335},	/* 665 */
	{16,8,8,8,L336},	/* 666 */
	{16,11,8,8,L336},	/* 667 */

	{8,8,16,8,L337},	/* 668 */
	{8,8,16,11,L337},	/* 669 */
	{16,8,16,9,L338},	/* 670 */
	{16,11,16,9,L338},	/* 671 */
	{16,8,16,8,L339},	/* 672 */
	{16,8,16,11,L339},	/* 673 */
	{16,11,16,8,L339},	/* 674 */
	{16,11,16,11,L339},	/* 675 */
	{127,8,4,0,L340},	/* 676 */
	{127,11,4,0,L340},	/* 677 */
	{127,8,8,0,L341},	/* 678 */
	{127,11,8,0,L341},	/* 679 */
	{127,8,16,9,L342},	/* 680 */
	{127,11,16,9,L342},	/* 681 */
	{127,8,16,8,L343},	/* 682 */
	{127,8,16,11,L343},	/* 683 */
	{127,11,16,8,L343},	/* 684 */
	{127,8,16,11,L343},	/* 685 */
	{63,8,4,0,L344},	/* 686 */
	{63,11,4,0,L344},	/* 687 */
	{63,8,8,0,L345},	/* 688 */
	{63,11,8,0,L345},	/* 689 */
	{63,8,8,8,L346},	/* 690 */
	{63,11,8,8,L346},	/* 691 */
	{63,8,16,9,L347},	/* 692 */
	{63,11,16,9,L347},	/* 693 */
	{63,8,16,8,L348},	/* 694 */
	{63,8,16,11,L348},	/* 695 */
	{63,11,16,8,L348},	/* 696 */
	{63,11,16,11,L348},	/* 697 */
	{127,8,84,8,L349},	/* 698 */
	{127,8,84,11,L349},	/* 699 */
	{127,11,84,8,L349},	/* 700 */
	{127,11,84,11,L349},	/* 701 */
	{63,8,84,8,L350},	/* 702 */
	{63,8,84,11,L350},	/* 703 */
	{63,11,84,8,L350},	/* 704 */
	{63,11,84,11,L350},	/* 705 */
	{63,8,63,8,L351},	/* 706 */
	{63,8,63,11,L351},	/* 707 */
	{63,11,63,8,L351},	/* 708 */
	{63,11,63,11,L351},	/* 709 */
/* & as in "if ((a&b) ==0)" */
	{0},
/* cc81 */
	{16,3,8,0,L352},	/* 711 */
	{16,10,8,0,L352},	/* 712 */
	{63,0,8,0,L353},	/* 713 */
	{16,0,20,0,L354},	/* 714 */
	{16,10,20,0,L354},	/* 715 */
	{127,0,16,0,L355},	/* 716 */
	{127,10,16,0,L355},	/* 717 */


	{63,0,16,1,L356},	/* 718 */


	{63,0,20,0,L357},	/* 719 */


	{63,0,63,0,L358},	/* 720 */


/* set codes right by moving the result */
	{0},
/* rest */
	{63,0,63,0,L359},	/* 722 */
	{63,4,63,4,L359},	/* 723 */


/* load */
	{0},
/* cs106 */
	{4,0,63,0,L360},	/* 725 */
	{4,4,63,0,L360},	/* 726 */
	{8,0,63,0,L361},	/* 727 */
	{16,3,63,0,L362},	/* 728 */
	{16,10,63,0,L362},	/* 729 */
	{16,1,63,0,L363},	/* 730 */
	{127,1,63,0,L364},	/* 731 */
	{8,8,63,0,L365},	/* 732 */
	{16,8,63,0,L366},	/* 733 */
	{16,11,63,0,L366},	/* 734 */
/* +1, +2, -1, -2 */
	{0},
/* cs91 */
	{63,0,5,0,L367},	/* 736 */
	{63,0,6,0,L368},	/* 737 */
/* +, -, |, &~ */
	{0},
/* cs40 */
	{16,0,8,0,L369},	/* 739 */
	{16,10,8,0,L369},	/* 740 */
	{63,0,8,0,L370},	/* 741 */
	{63,10,8,0,L370},	/* 742 */
	{16,0,16,1,L371},	/* 743 */
	{16,0,127,1,L372},	/* 744 */
	{16,0,63,0,L373},	/* 745 */
/* integer to long */
	{0},
/* cs58 */
	{8,0,63,0,L374},	/* 747 */
	{63,9,63,0,L375},	/* 748 */
	{16,1,63,0,L376},	/* 749 */
/* float to long */
	{0},
/* cs56 */
	{63,4,63,0,L377},	/* 751 */
/* setup for structure assign */
	{0},
/* ci116 */
	{63,0,20,0,L378},	/* 753 */
	{63,0,63,0,L379},	/* 754 */
/* end of table */
	{0},
};

/*
 * c code tables-- compile to register
 */

struct table regtab[] = {
	{106,cr106},	/* load */
	{30,cr70},	/* prefix ++, handled as += */
	{31,cr70},	/* prefix --, handled as -= */
	{32,cr32},	/* postfix ++ */
	{33,cr32},	/* postfix -- */
	{37,cr37},	/* unary - */
	{38,cr37},	/* ~ */
	{98,cr100},	/* call */
	{99,cr100},	/* call */
	{80,cr80},	/* = */
	{91,cr91},	/* +1, +2 */
	{92,cr91},	/* -1, -2 */
	{40,cr40},	/* + */
	{41,cr40},	/* - */
	{42,cr42},	/* *, char/int, both signed & unsigned */
	{43,cr43},	/* /, signed char/int, calls @idiv */
	{14,cr14},	/* PTOI: scale pointer difference to int */
	{44,cr43},	/* %, signed char/int, calls @irem */
	{45,cr45},	/* >>, signed */
	{17,cr45},	/* >>, unsigned */
	{46,cr45},	/* << */
	{55,cr40},	/* &~ */
	{48,cr40},	/* | */
	{49,cr49},	/* ^ */
	{70,cr70},	/* += */
	{71,cr70},	/* -= */
	{72,cr72},	/* *= */
	{73,cr73},	/* /=, signed char/int, calls @idiv */
	{74,cr73},	/* %=, signed char/int, calls @irem */
	{75,cr75},	/* >>=, signed */
	{18,cr75},	/* >>=, unsigned */
	{76,cr75},	/* <<= */
	{78,cr78},	/* |= */
	{85,cr78},	/* &= */
	{79,cr79},	/* ^= */
	{102,cr102},	/* goto */
	{51,cr51},	/* cvt int => float */
	{52,cr52},	/* cvt float => int */
	{56,cr56},	/* cvt long => float */
	{57,cr57},	/* cvt float => long */
	{58,cr58},	/* cvt int => long */
	{59,cr59},	/* cvt long => int */
	{82,cr82},	/* *, signed long, calls @lmul */
	{83,cr82},	/* /, signed long, calls @ldiv */
	{84,cr82},	/* %, signed long, calls @lrem */
	{86,cr86},	/* *=, signed long, calls @almul */
	{87,cr86},	/* /=, signed long, calls @aldiv */
	{88,cr86},	/* %=, signed long, calls @alrem */
	{16,cr16},	/* = for bit fields */
	{109,cr109},	/* cvt int -> char */
	{117,cr117},	/* /  for unsigned char/int or divisor known positive */
	{118,cr117},	/* %  for unsigned char/int or divisor known positive */
	{119,cr119},	/* /= for unsigned char/int or divisor known positive */
	{120,cr119},	/* %= for unsigned char/int or divisor known positive */
	{107,cr107},	/* special case (int *) - (int *) */
	{121,cr121},	/* *, unsigned long, calls @lmul */
	{122,cr121},	/* /, unsigned long, calls @uldiv */
	{123,cr121},	/* %, unsigned long, calls @ulrem */
	{124,cr124},	/* *=, unsigned long, calls @almul */
	{125,cr124},	/* /=, unsigned long, calls @ualdiv */
	{126,cr124},	/* %=, unsigned long, calls @ualrem */
	{127,cr127},	/* cvt unsigned long to float, calls @ultof */
	{130,cr130},	/* special handling of 'x - &name' */
	{0}
};

/*
 * c code tables -- compile for side effects.
 * Also set condition codes properly (except for ++, --)
 */

struct table efftab[] = {
	{30,ci70},	/* prefix ++ */
	{31,ci70},	/* prefix -- */
	{32,ci70},	/* postfix ++ */
	{33,ci70},	/* postfix -- */
	{80,ci80},	/* = */
	{70,ci70},	/* += */
	{71,ci70},	/* -= */
	{78,ci78},	/* |= */
	{79,ci79},	/* ^= */
	{85,ci78},	/* &~= */
	{16,ci16},	/* field assign, FSELA */
	{116,ci116},	/* structure assignment setup */
	{0}
};

/*
 * c code tables-- set condition codes
 */

struct table cctab[] = {
	{106,cc60},	/* load */
	{55,rest},	/* &~ */
	{34,rest},	/* ! */
	{35,rest},	/* & */
	{36,rest},	/* * */
	{37,rest},	/* unary - */
	{40,rest},	/* + */
	{41,rest},	/* - */
	{43,rest},	/* / */
	{81,cc81},	/* & as in "if ((a&b)==0)" */
	{48,rest},	/* | */
	{60,cc60},	/* == */
	{61,cc60},	/* =! */
	{62,cc60},	/* <=, signed */
	{63,cc60},	/* <, signed */
	{64,cc60},	/* >=, signed */
	{65,cc60},	/* >, signed */
	{66,cc60},	/* <=, unsigned */
	{67,cc60},	/* <, unsigned */
	{68,cc60},	/* >=, unsigned */
	{69,cc60},	/* > , unsigned */
	{72,rest},	/* *= */
	{73,rest},	/* /= */
	{79,rest},	/* ^= */
	{0}
};

/*
 * c code tables-- expression to -(sp)
 */

struct table sptab[] = {
	{106,cs106},		/* load */
	{91,cs91},		/* +1, +2 */
	{92,cs91},		/* -1, -2 */
	{40,cs40},		/* + */
	{41,cs40},		/* - */
	{55,cs40},		/* &~ */
	{48,cs40},		/* | */
	{58,cs58},		/* cvt int->long */
	{56,cs56},		/* cvt float->long */
	{0}
};
