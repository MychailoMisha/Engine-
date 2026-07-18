#include <QtWidgets>

enum Roles { NameRole = Qt::UserRole + 1, TypeRole, ScriptRole, ColorRole, AssetRole, FitRole, WidthRole, HeightRole, FontRole, TargetSceneRole,
             CellSizeRole, ColumnsRole, RowsRole, GridVisibleRole, FillColorRole, StrokeColorRole, GravityRole, LockRotationRole,
             CameraFollowRole, CameraAdaptiveRole, CullMarginRole, MoveUpRole, MoveDownRole, MoveLeftRole, MoveRightRole, HitboxRole, ShowHitboxRole,
             TextRole, SceneRole, ShapeRole, ZRole, RotationRole, ScaleRole, OpacityRole, ObjectVisibleRole, BehaviorRole, BlurRole, PhysicsRole,
             GlassRole, TextOpacityRole, CellColorsRole, CellPaintColorRole, CellPaintModeRole, TileCellsRole, TileIndexRole, UiStateRole, TargetObjectRole, AltAssetRole, CornerRadiusRole, UiValueRole, PressColorRole, PressEffectRole,
             AutoPlayRole, LoopRole, VolumeRole, SaveKeyRole, SaveFieldsRole, AutoLoadRole, VibrationDurationRole, VibrationOnStartRole, RadioGroupRole };

class CppHighlighter final : public QSyntaxHighlighter {
  struct Rule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  QVector<Rule> rules;

public:
  explicit CppHighlighter(QTextDocument *doc) : QSyntaxHighlighter(doc) {
    auto add = [this](QString p, QColor c, bool bold = false) {
      QTextCharFormat f;
      f.setForeground(c);
      if (bold)
        f.setFontWeight(QFont::Bold);
      rules.push_back({QRegularExpression(p), f});
    };
    add(R"(\b(auto|bool|break|case|catch|char|class|const|continue|default|delete|do|double|else|enum|explicit|false|float|for|if|int|long|namespace|new|nullptr|private|protected|public|return|short|signed|sizeof|static|struct|switch|template|this|throw|true|try|typedef|typename|unsigned|using|virtual|void|while)\b)",
        "#c792ea", true);
    add(R"(\b(Scene|Entity|Input|QString|QColor)\b)", "#62d8f1", true);
    add(R"(\b(onStart|onUpdate|find|findType)\b)", "#82d46d", true);
    add(R"(\b[0-9]+(?:\.[0-9]+)?f?\b)", "#f5b971");
    add(R"("(?:\\.|[^"\\])*")", "#c3e88d");
    add(R"(//[^\n]*)", "#6f7f9f");
    add(R"(^\s*#\s*\w+)", "#ffcb6b", true);
  }

protected:
  void highlightBlock(const QString &text) override {
    for (const auto &r : rules) {
      auto it = r.pattern.globalMatch(text);
      while (it.hasNext()) {
        auto m = it.next();
        setFormat(m.capturedStart(), m.capturedLength(), r.format);
      }
    }
  }
};

class CodeEditor final : public QPlainTextEdit {
public:
  explicit CodeEditor(const QString &text) : QPlainTextEdit(text) {
    new CppHighlighter(document());
  }
};
#define QPlainTextEdit CodeEditor

class EntityItem final : public QGraphicsRectItem {
  bool resizing=false;
public:
  void paint(QPainter *p,const QStyleOptionGraphicsItem *option,QWidget*) override {
    p->setRenderHint(QPainter::Antialiasing);p->setRenderHint(QPainter::SmoothPixmapTransform);p->setRenderHint(QPainter::LosslessImageRendering);QString type=data(TypeRole).toString();QRectF box=rect();QColor color=data(ColorRole).value<QColor>();qreal radius=qMax(0.0,data(CornerRadiusRole).toDouble());
    QString image=data(AssetRole).toString();if(!image.isEmpty()&&type!="Tile Map"&&type!="Theme Switch"){QPixmap src(image);if(!src.isNull()){double blur=data(BlurRole).toDouble();if(blur>0){int divisor=qMax(2,1+qRound(blur/12));src=src.scaled(qMax(1,src.width()/divisor),qMax(1,src.height()/divisor),Qt::IgnoreAspectRatio,Qt::SmoothTransformation).scaled(src.size(),Qt::IgnoreAspectRatio,Qt::SmoothTransformation);}QString fit=data(FitRole).toString();p->save();QPainterPath clip;clip.addRoundedRect(box,radius,radius);p->setClipPath(clip);if(fit=="Center")p->drawPixmap(QPointF(box.center().x()-src.width()/2,box.center().y()-src.height()/2),src);else{Qt::AspectRatioMode mode=fit=="Fill"?Qt::IgnoreAspectRatio:(fit=="Crop"?Qt::KeepAspectRatioByExpanding:Qt::KeepAspectRatio);QPixmap scaled=src.scaled(box.size().toSize(),mode,Qt::SmoothTransformation);p->drawPixmap(QPointF(box.center().x()-scaled.width()/2,box.center().y()-scaled.height()/2),scaled);}p->restore();}}
    else if(type=="Tile Map"){int cell=qMax(1,data(CellSizeRole).toInt()),cols=data(ColumnsRole).toInt(),rows=data(RowsRole).toInt();QPixmap tileset(image);QVariantMap placed=data(TileCellsRole).toMap();p->fillRect(box,data(FillColorRole).value<QColor>());int sourceCols=tileset.isNull()?1:qMax(1,tileset.width()/cell);for(auto it=placed.begin();it!=placed.end();++it){QStringList xy=it.key().split(',');if(xy.size()!=2||tileset.isNull())continue;int index=it.value().toInt(),sx=(index%sourceCols)*cell,sy=(index/sourceCols)*cell;p->drawPixmap(QRectF(box.left()+xy[0].toInt()*cell,box.top()+xy[1].toInt()*cell,cell,cell),tileset,QRectF(sx,sy,cell,cell));}if(data(GridVisibleRole).toBool()){p->setPen(QPen(data(StrokeColorRole).value<QColor>(),1));for(int x=0;x<=cols;x++)p->drawLine(box.left()+x*cell,box.top(),box.left()+x*cell,box.top()+rows*cell);for(int y=0;y<=rows;y++)p->drawLine(box.left(),box.top()+y*cell,box.left()+cols*cell,box.top()+y*cell);}}
    else if(type=="Text"){p->setPen(color);p->setFont(QFont(data(FontRole).toString(),20,QFont::Bold));p->drawText(box,Qt::AlignCenter,data(TextRole).toString());}
    else if(type=="Button"){p->setPen(QPen(color.lighter(145),2));p->setBrush(color.darker(155));p->drawRoundedRect(box,radius,radius);p->setPen(Qt::white);p->setFont(QFont(data(FontRole).toString(),16,QFont::Bold));p->drawText(box,Qt::AlignCenter,data(TextRole).toString());}
    else if(type=="Button Container"||type=="UI Container"){QStringList labels=data(TextRole).toString().split(',',Qt::SkipEmptyParts);if(labels.isEmpty())labels={"Play","Settings","Exit"};bool vertical=data(ShapeRole).toString()!="Horizontal";p->setPen(QPen(color.lighter(135),2));p->setBrush(QColor(18,25,42,210));p->drawRoundedRect(box,14,14);for(int i=0;i<labels.size();++i){QRectF b=vertical?QRectF(box.left()+10,box.top()+10+i*(box.height()-20)/labels.size(),box.width()-20,(box.height()-20)/labels.size()-6):QRectF(box.left()+10+i*(box.width()-20)/labels.size(),box.top()+10,(box.width()-20)/labels.size()-6,box.height()-20);p->setBrush(color.darker(145));p->setPen(QPen(color.lighter(140),1));p->drawRoundedRect(b,8,8);p->setPen(Qt::white);p->drawText(b,Qt::AlignCenter,labels[i].trimmed());}}
    else if(type=="Dropdown"){p->setPen(QPen(color.lighter(145),2));p->setBrush(color.darker(165));p->drawRoundedRect(box,radius,radius);p->setPen(Qt::white);QString label=data(TextRole).toString().split(',',Qt::SkipEmptyParts).value(0,"Select");p->drawText(box.adjusted(12,0,-34,0),Qt::AlignVCenter|Qt::AlignLeft,label.trimmed());p->drawText(box.adjusted(box.width()-34,0,-8,0),Qt::AlignCenter,"▼");}
    else if(type=="Toggle"){p->setPen(Qt::NoPen);p->setBrush(data(UiStateRole).toBool()?QColor("#28d7b8"):QColor("#3a4358"));p->drawRoundedRect(box,box.height()/2,box.height()/2);qreal d=box.height()-10;p->setBrush(Qt::white);p->drawEllipse(QRectF(data(UiStateRole).toBool()?box.right()-d-5:box.left()+5,box.top()+5,d,d));p->setPen(Qt::white);p->drawText(box.adjusted(8,0,-8,0),data(UiStateRole).toBool()?Qt::AlignLeft|Qt::AlignVCenter:Qt::AlignRight|Qt::AlignVCenter,data(UiStateRole).toBool()?"ON":"OFF");}
    else if(type=="Theme Switch"){bool dark=data(UiStateRole).toBool();p->setPen(QPen(dark?QColor("#7f8bad"):QColor("#d7dcef"),2));p->setBrush(dark?QColor("#171c2b"):QColor("#f5f7ff"));p->drawRoundedRect(box,box.height()/2,box.height()/2);p->setPen(dark?Qt::white:QColor("#22293a"));p->drawText(box,Qt::AlignCenter,dark?"☾  DARK":"☀  LIGHT");}
    else if(type=="Slider"||type=="Progress Bar"){int value=qBound(0,data(UiValueRole).toInt(),100);p->setPen(Qt::NoPen);p->setBrush(QColor("#303a52"));p->drawRoundedRect(box,radius,radius);QRectF fill=box;fill.setWidth(box.width()*value/100.0);p->setBrush(color);p->drawRoundedRect(fill,radius,radius);if(type=="Slider"){p->setBrush(Qt::white);p->drawEllipse(QPointF(box.left()+box.width()*value/100.0,box.center().y()),qMax(7.0,box.height()*.32),qMax(7.0,box.height()*.32));}else{p->setPen(Qt::white);p->drawText(box,Qt::AlignCenter,QString::number(value)+"%");}}
    else if(type=="Check Box"){bool checked=data(UiStateRole).toBool();QRectF check(box.left()+5,box.center().y()-13,26,26);p->setBrush(checked?color:QColor("#242c40"));p->setPen(QPen(color.lighter(145),2));p->drawRoundedRect(check,6,6);if(checked){p->setPen(QPen(Qt::white,3));p->drawLine(check.left()+6,check.center().y(),check.center().x()-1,check.bottom()-6);p->drawLine(check.center().x()-1,check.bottom()-6,check.right()-5,check.top()+6);}p->setPen(Qt::white);p->drawText(box.adjusted(40,0,0,0),Qt::AlignVCenter|Qt::AlignLeft,data(TextRole).toString());}
    else if(type=="Radio Button"){bool checked=data(UiStateRole).toBool();QRectF radio(box.left()+7,box.center().y()-12,24,24);p->setPen(QPen(checked?color:QColor("#66718c"),2));p->setBrush(QColor("#111827"));p->drawEllipse(radio);if(checked){p->setPen(Qt::NoPen);p->setBrush(color);p->drawEllipse(radio.adjusted(6,6,-6,-6));}p->setPen(Qt::white);p->drawText(box.adjusted(42,0,0,0),Qt::AlignVCenter|Qt::AlignLeft,data(TextRole).toString());}
    else if(type=="Video Player"||type=="Audio Source"||type=="Vibration"||type=="Save Data"){QString icon=type=="Video Player"?"▶":(type=="Audio Source"?"♫":(type=="Vibration"?"≋":"▣"));p->setPen(QPen(color.lighter(145),2));p->setBrush(QColor("#111a2c"));p->drawRoundedRect(box,14,14);p->setPen(Qt::white);p->setFont(QFont("Segoe UI",18,QFont::Bold));p->drawText(box.adjusted(12,4,-12,-24),Qt::AlignCenter,icon);p->setFont(QFont("Segoe UI",10,QFont::DemiBold));p->drawText(box.adjusted(8,26,-8,-6),Qt::AlignCenter,type);}
    else if(type=="Scroll View"){p->setPen(QPen(color.lighter(130),2));p->setBrush(QColor("#111827"));p->drawRoundedRect(box,radius,radius);p->save();p->setClipRect(box.adjusted(8,8,-14,-8));QStringList items=data(TextRole).toString().split(',',Qt::SkipEmptyParts);p->setPen(Qt::white);for(int i=0;i<items.size();++i)p->drawText(QRectF(box.left()+14,box.top()+12+i*30,box.width()-34,26),Qt::AlignVCenter|Qt::AlignLeft,items[i].trimmed());p->restore();p->setBrush(QColor("#596780"));p->setPen(Qt::NoPen);p->drawRoundedRect(QRectF(box.right()-9,box.top()+10,4,qMax(28.0,box.height()*.32)),2,2);}
    else if(type=="Character"){p->setPen(QPen(color.lighter(150),2));p->setBrush(color.darker(165));p->drawRoundedRect(box,10,10);}
    else if(type=="Light"){QRadialGradient light(box.center(),qMax(box.width(),box.height())/2);QColor core=color;core.setAlpha(210);QColor edge=color;edge.setAlpha(0);light.setColorAt(0,core);light.setColorAt(1,edge);p->setPen(Qt::NoPen);p->setBrush(light);p->drawEllipse(box);}
    else if(type=="Particle"){p->setPen(Qt::NoPen);p->setBrush(color);for(int k=0;k<12;k++){double a=k*.52;p->drawEllipse(box.center()+QPointF(std::cos(a)*box.width()*.4,std::sin(a)*box.height()*.4),3,3);}}
    else if(type=="Color Block"){double blur=data(BlurRole).toDouble();p->setPen(blur>0?Qt::NoPen:QPen(color.lighter(140),2));if(blur>0){QRadialGradient soft(box.center(),qMax(box.width(),box.height())*.72);QColor edge=color;edge.setAlpha(0);soft.setColorAt(0,color);soft.setColorAt(qBound(0.05,1.0-blur/110.0,0.95),color);soft.setColorAt(1,edge);p->setBrush(soft);}else p->setBrush(color);p->drawRoundedRect(box,12,12);}
    else if(type=="Physics Block"){p->setPen(QPen(color.lighter(155),3));p->setBrush(color.darker(155));p->drawRoundedRect(box,7,7);p->drawLine(box.topLeft(),box.bottomRight());p->drawLine(box.topRight(),box.bottomLeft());}
    else if(type=="Panel"||type=="Glass Panel"){bool glass=data(GlassRole).toBool()||type=="Glass Panel";QColor panel=color;panel.setAlpha(glass?72:210);QLinearGradient g(box.topLeft(),box.bottomRight());g.setColorAt(0,panel.lighter(145));g.setColorAt(1,panel.darker(125));p->setBrush(g);p->setPen(QPen(glass?QColor(255,255,255,125):color.lighter(135),glass?2:1));p->drawRoundedRect(box,18,18);if(glass){p->setPen(QPen(QColor(255,255,255,80),2));p->drawLine(box.topLeft()+QPointF(18,10),box.topRight()+QPointF(-18,10));}}
    else if(type!="Grid Map"&&type!="ASFO Territory"&&type!="Camera"){p->setPen(QPen(color.lighter(150),2));p->setBrush(color.darker(170));p->drawRect(box);}
    if(data(GlassRole).toBool()&&type!="Light"&&type!="Camera"){p->setPen(QPen(QColor(255,255,255,125),1.5));p->setBrush(QColor(220,235,255,38));p->drawRoundedRect(box,radius,radius);p->setPen(QPen(QColor(255,255,255,70),2));p->drawLine(box.topLeft()+QPointF(radius+6,7),box.topRight()+QPointF(-radius-6,7));}if(data(BlurRole).toDouble()>0&&image.isEmpty()&&type!="Light"){QColor glow=color;glow.setAlpha(qBound(12,qRound(data(BlurRole).toDouble()*0.7),70));p->setBrush(Qt::NoBrush);for(int k=1;k<=3;k++){p->setPen(QPen(glow,k*2));p->drawRoundedRect(box.adjusted(-k,-k,k,k),radius+k,radius+k);}}
    if(option->state&QStyle::State_Selected){p->setBrush(Qt::NoBrush);p->setPen(QPen(QColor("#ffffff"),2,Qt::DashLine));p->drawRect(box.adjusted(-3,-3,3,3));p->setPen(QPen(QColor("#171d2f"),1));p->setBrush(QColor("#ffffff"));for(const QPointF&h:{box.topLeft(),box.topRight(),box.bottomLeft(),box.bottomRight(),QPointF(box.center().x(),box.top()),QPointF(box.center().x(),box.bottom()),QPointF(box.left(),box.center().y()),QPointF(box.right(),box.center().y())})p->drawRect(QRectF(h-QPointF(5,5),QSizeF(10,10)));}
  }
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *e) override {QRectF box=rect();QString type=data(TypeRole).toString();if((type=="Grid Map"||type=="Tile Map")&&data(CellPaintModeRole).toBool()&&e->button()==Qt::LeftButton){int cell=qMax(1,data(CellSizeRole).toInt()),x=qFloor((e->pos().x()-box.left())/cell),y=qFloor((e->pos().y()-box.top())/cell);if(x>=0&&y>=0&&x<data(ColumnsRole).toInt()&&y<data(RowsRole).toInt()){QString key=QString::number(x)+","+QString::number(y);if(type=="Tile Map"){QVariantMap cells=data(TileCellsRole).toMap();cells[key]=data(TileIndexRole);setData(TileCellsRole,cells);}else{QVariantMap colors=data(CellColorsRole).toMap();colors[key]=data(CellPaintColorRole);setData(CellColorsRole,colors);}update();e->accept();return;}}bool onHandle=false;for(const QPointF&h:{box.topLeft(),box.topRight(),box.bottomLeft(),box.bottomRight(),QPointF(box.center().x(),box.top()),QPointF(box.center().x(),box.bottom()),QPointF(box.left(),box.center().y()),QPointF(box.right(),box.center().y())})if(QLineF(h,e->pos()).length()<=12){onHandle=true;break;}if(isSelected()&&onHandle){resizing=true;e->accept();return;}QGraphicsRectItem::mousePressEvent(e);}
  void mouseMoveEvent(QGraphicsSceneMouseEvent *e) override {if(!resizing){QGraphicsRectItem::mouseMoveEvent(e);if(scene())scene()->update();return;}qreal w=qMax(8.0,2.0*qAbs(e->pos().x())),h=qMax(8.0,2.0*qAbs(e->pos().y()));if(data(TypeRole)=="Grid Map"||data(TypeRole)=="Tile Map"){int cell=qMax(1,data(CellSizeRole).toInt());w=qMax((qreal)cell,(qreal)(qRound(w/cell)*cell));h=qMax((qreal)cell,(qreal)(qRound(h/cell)*cell));setData(ColumnsRole,qMax(1,qRound(w/cell)));setData(RowsRole,qMax(1,qRound(h/cell)));}setRect(-w/2,-h/2,w,h);setData(WidthRole,w);setData(HeightRole,h);update();if(scene())scene()->update();e->accept();}
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *e) override {if(resizing){resizing=false;e->accept();return;}QGraphicsRectItem::mouseReleaseEvent(e);}
};

class SceneView final : public QGraphicsView {
public:
  QGraphicsScene world;
  bool snapEnabled=true;
  int snapSize=32;
  bool portraitMode=false;
  std::function<void()> changed;
  std::function<void()> requestSave;
  std::function<void(QGraphicsRectItem*)> requestImage;
  SceneView() {
    setScene(&world);
    world.setSceneRect(-900, -550, 1800, 1100);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setDragMode(RubberBandDrag);
    setTransformationAnchor(AnchorUnderMouse);
    setBackgroundBrush(QColor("#090d16"));
    add("Character", {-100, 30});
    add("Camera", {0, 0});
    add("Light", {-260, -150});
    add("Platform", {-30, 180});
  }
  static QColor typeColor(const QString &t) {
    if (t == "Character")
      return QColor("#7c5cff");
    if (t == "Camera")
      return QColor("#28d7c0");
    if (t == "Light")
      return QColor("#ffbd4a");
    if (t == "Shader")
      return QColor("#ff617e");
    if (t == "Particle")
      return QColor("#53a7ff");
    if (t == "Collider")
      return QColor("#94a0bf");
    return QColor("#6f7f9f");
  }
  void add(const QString &type, QPointF p = {}) {
    QColor c = typeColor(type);
    auto *r = new EntityItem;world.addItem(r);r->setRect(-30,-30,60,60);r->setPen(QPen(c.lighter(150),2));r->setBrush(QBrush(c.darker(170)));
    r->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    r->setData(NameRole, type + " " + QString::number(itemsOfType(type) + 1));
    r->setData(TypeRole, type);
    r->setData(ScriptRole, type == "Character" ? "Player Controller" : "None");
    r->setData(BehaviorRole, type == "Character" ? "Player Controller" : "None");
    r->setData(ColorRole, c);
    r->setData(AssetRole, "");
    r->setData(FitRole, "Fit");
    r->setData(WidthRole, 60.0);
    r->setData(HeightRole, 60.0);
    r->setData(FontRole, "Segoe UI");
    r->setData(TargetSceneRole, "Scene 1");
    r->setData(CellSizeRole, 32);
    r->setData(ColumnsRole, 25);
    r->setData(RowsRole, 15);
    r->setData(GridVisibleRole, true);
    r->setData(FillColorRole, QColor("#172033"));
    r->setData(StrokeColorRole, QColor("#35405c"));
    r->setData(GravityRole, type == "Character" ? 900.0 : 0.0);
    r->setData(LockRotationRole, type == "Character");
    r->setData(CameraFollowRole, "None");
    r->setData(CameraAdaptiveRole, true);
    r->setData(CullMarginRole, 64);
    r->setData(MoveUpRole, "W"); r->setData(MoveDownRole, "S");
    r->setData(MoveLeftRole, "A"); r->setData(MoveRightRole, "D");
    r->setData(HitboxRole, type=="Character"||type=="Platform"||type=="Solid Ground"||type=="Collider");
    r->setData(ShowHitboxRole, false);
    r->setData(TextRole, type=="Button"?"Button":(type=="Text"?"Text label":""));
    r->setData(SceneRole, "Scene 1");
    r->setData(ShapeRole, "Circle");
    r->setData(BlurRole,0.0);r->setData(PhysicsRole,type=="Character"||type=="Physics Block");
    r->setData(GlassRole,type=="Glass Panel");r->setData(TextOpacityRole,true);r->setData(CellColorsRole,QVariantMap());r->setData(CellPaintColorRole,QColor("#ff5d8f"));r->setData(CellPaintModeRole,false);r->setData(TileCellsRole,QVariantMap());r->setData(TileIndexRole,0);r->setData(UiStateRole,false);r->setData(TargetObjectRole,"");r->setData(AltAssetRole,"");r->setData(CornerRadiusRole,10.0);r->setData(UiValueRole,50);r->setData(PressColorRole,QColor("#a997ff"));r->setData(PressEffectRole,true);
    r->setData(AutoPlayRole,true);r->setData(LoopRole,false);r->setData(VolumeRole,80);r->setData(SaveKeyRole,"MainSave");r->setData(SaveFieldsRole,"Player.x,Player.y,Score.text");r->setData(AutoLoadRole,true);r->setData(VibrationDurationRole,120);r->setData(VibrationOnStartRole,false);r->setData(RadioGroupRole,"Default");
    r->setData(ZRole, 0);r->setData(RotationRole,0.0);r->setData(ScaleRole,1.0);r->setData(OpacityRole,1.0);r->setData(ObjectVisibleRole,true);
    if(type=="Background Image"){r->setData(WidthRole,1280.0);r->setData(HeightRole,720.0);r->setData(FitRole,"Fill");r->setData(HitboxRole,false);r->setData(ZRole,-900);r->setZValue(-900);r->setRect(-640,-360,1280,720);}
    if(type=="Color Block"||type=="Physics Block"){r->setData(WidthRole,140.0);r->setData(HeightRole,90.0);r->setRect(-70,-45,140,90);if(type=="Physics Block"){r->setData(GravityRole,900.0);r->setData(HitboxRole,true);}}
    if(type=="Panel"||type=="Glass Panel"){r->setData(WidthRole,320.0);r->setData(HeightRole,180.0);r->setData(HitboxRole,false);r->setRect(-160,-90,320,180);}
    if(type=="Button Container"||type=="UI Container"){r->setData(WidthRole,300.0);r->setData(HeightRole,220.0);r->setData(TextRole,"Play, Settings, Exit");r->setData(ShapeRole,"Vertical");r->setData(HitboxRole,false);r->setRect(-150,-110,300,220);}
    if(type=="Dropdown"){r->setData(WidthRole,240.0);r->setData(HeightRole,52.0);r->setData(TextRole,"Easy, Normal, Hard");r->setData(HitboxRole,false);r->setRect(-120,-26,240,52);}
    if(type=="Toggle"){r->setData(WidthRole,120.0);r->setData(HeightRole,48.0);r->setData(TextRole,"Toggle");r->setData(HitboxRole,false);r->setRect(-60,-24,120,48);}
    if(type=="Theme Switch"){r->setData(WidthRole,160.0);r->setData(HeightRole,52.0);r->setData(TextRole,"THEME");r->setData(ColorRole,QColor("#f5f7ff"));r->setData(FillColorRole,QColor("#171c2b"));r->setData(HitboxRole,false);r->setRect(-80,-26,160,52);}
    if(type=="Slider"||type=="Progress Bar"){r->setData(WidthRole,260.0);r->setData(HeightRole,34.0);r->setData(HitboxRole,false);r->setRect(-130,-17,260,34);}
    if(type=="Check Box"){r->setData(WidthRole,180.0);r->setData(HeightRole,42.0);r->setData(TextRole,"Option");r->setData(HitboxRole,false);r->setRect(-90,-21,180,42);}
    if(type=="Radio Button"){r->setData(WidthRole,180.0);r->setData(HeightRole,42.0);r->setData(TextRole,"Choice");r->setData(HitboxRole,false);r->setRect(-90,-21,180,42);}
    if(type=="Save Data"){r->setData(WidthRole,170.0);r->setData(HeightRole,70.0);r->setData(HitboxRole,false);r->setRect(-85,-35,170,70);}
    if(type=="Vibration"){r->setData(WidthRole,150.0);r->setData(HeightRole,60.0);r->setData(HitboxRole,false);r->setRect(-75,-30,150,60);}
    if(type=="Scroll View"){r->setData(WidthRole,320.0);r->setData(HeightRole,240.0);r->setData(TextRole,"Item 1, Item 2, Item 3, Item 4, Item 5");r->setData(HitboxRole,false);r->setData(CornerRadiusRole,16.0);r->setRect(-160,-120,320,240);}
    if(type=="Camera"){r->setData(WidthRole,800.0);r->setData(HeightRole,450.0);r->setData(ShapeRole,"Follow Game");r->setRect(-30,-30,60,60);}
    if(type=="Grid Map"||type=="Tile Map"){r->setData(WidthRole,800.0);r->setData(HeightRole,480.0);r->setRect(0,0,800,480);r->setBrush(Qt::NoBrush);}
    else if(type=="ASFO Territory"){r->setData(WidthRole,1200.0);r->setData(HeightRole,1200.0);r->setRect(-600,-600,1200,1200);r->setBrush(Qt::NoBrush);}
    r->setPos(p.isNull() ? mapToScene(viewport()->rect().center()) : p);
    auto *l = new QGraphicsSimpleTextItem(r->data(NameRole).toString(), r);
    l->setBrush(Qt::white);
    l->setPos(-l->boundingRect().width() / 2, 38);
    l->setVisible(type!="Button"&&type!="Text"&&type!="Background Image");
    if (changed)
      changed();
  }
  QList<QGraphicsRectItem *> entities() const {
    QList<QGraphicsRectItem *> a;
    for (auto *i : world.items(Qt::AscendingOrder))
      if (auto *r = qgraphicsitem_cast<QGraphicsRectItem *>(i))
        a << r;
    return a;
  }
  int itemsOfType(const QString &t) const {
    int n = 0;
    for (auto *r : entities())
      n += r->data(TypeRole) == t;
    return n;
  }

protected:
  void contextMenuEvent(QContextMenuEvent *event) override {
    QGraphicsItem *hit=itemAt(event->pos());while(hit&&hit->parentItem())hit=hit->parentItem();
    auto *r=qgraphicsitem_cast<QGraphicsRectItem*>(hit);
    QMenu menu(this);
    if(!r){auto *save=menu.addAction("Save Project");if(menu.exec(event->globalPos())==save&&requestSave)requestSave();return;}
    world.clearSelection();r->setSelected(true);
    auto *layers=menu.addMenu("Layer / Draw Order");
    auto *front=layers->addAction("Bring to Front");auto *forward=layers->addAction("Move Forward");
    auto *backward=layers->addAction("Move Backward");auto *back=layers->addAction("Send to Back");
    auto *movement=menu.addMenu("Movement / Behavior");QList<QAction*> movementActions;
    for(const QString &name:{"None","Player Controller","Patrol","Rotator","Floating","Bounce","Orbit","Blink","Pulse Scale"}){auto*a=movement->addAction(name);a->setCheckable(true);a->setChecked(r->data(BehaviorRole).toString()==name);movementActions<<a;}
    menu.addSeparator();auto *image=menu.addAction("Choose / Change Image...");auto *duplicate=menu.addAction("Duplicate");auto *reset=menu.addAction("Reset Rotation & Scale");
    auto *collision=menu.addAction(r->data(HitboxRole).toBool()?"Disable Hitbox":"Enable Hitbox");
    auto *visibility=menu.addAction(r->data(ObjectVisibleRole).toBool()?"Hide Object":"Show Object");
    menu.addSeparator();auto *save=menu.addAction("Save Project");auto *remove=menu.addAction("Delete Object");
    QAction *chosen=menu.exec(event->globalPos());if(!chosen)return;
    auto layer=[&](int z){z=qBound(-1000,z,1000);r->setData(ZRole,z);r->setZValue(z);};
    if(chosen==front)layer(1000);else if(chosen==forward)layer(r->data(ZRole).toInt()+1);else if(chosen==backward)layer(r->data(ZRole).toInt()-1);else if(chosen==back)layer(-1000);
    else if(movementActions.contains(chosen)){r->setData(BehaviorRole,chosen->text());r->setData(ScriptRole,chosen->text());}
    else if(chosen==image){if(requestImage)requestImage(r);}
    else if(chosen==reset){r->setData(RotationRole,0.0);r->setData(ScaleRole,1.0);r->setRotation(0);r->setScale(1);}
    else if(chosen==collision)r->setData(HitboxRole,!r->data(HitboxRole).toBool());
    else if(chosen==visibility){bool visible=!r->data(ObjectVisibleRole).toBool();r->setData(ObjectVisibleRole,visible);r->setVisible(visible);}
    else if(chosen==duplicate){auto *copy=new EntityItem;world.addItem(copy);copy->setRect(r->rect());copy->setPen(r->pen());copy->setBrush(r->brush());copy->setFlags(r->flags());for(int role=NameRole;role<=BehaviorRole;role++)copy->setData(role,r->data(role));copy->setData(NameRole,r->data(NameRole).toString()+" Copy");copy->setPos(r->pos()+QPointF(24,24));copy->setZValue(r->zValue());copy->setRotation(r->rotation());copy->setScale(r->scale());copy->setOpacity(r->opacity());auto*l=new QGraphicsSimpleTextItem(copy->data(NameRole).toString(),copy);l->setBrush(Qt::white);l->setPos(-l->boundingRect().width()/2,38);l->setVisible(copy->data(TypeRole)!="Button"&&copy->data(TypeRole)!="Text"&&copy->data(TypeRole)!="Background Image");world.clearSelection();copy->setSelected(true);}
    else if(chosen==save){if(requestSave)requestSave();return;}else if(chosen==remove){world.removeItem(r);delete r;}
    viewport()->update();if(changed)changed();
  }
  void drawBackground(QPainter *p, const QRectF &r) override {
    p->fillRect(r, QColor("#090d16"));
    QPen a(QColor("#1b2233"), 0), b(QColor("#29324a"), 0);
    int l = qFloor(r.left() / 32) * 32, t = qFloor(r.top() / 32) * 32;
    for (int x = l; x < r.right(); x += 32) {
      p->setPen(x % 160 ? a : b);
      p->drawLine(x, r.top(), x, r.bottom());
    }
    for (int y = t; y < r.bottom(); y += 32) {
      p->setPen(y % 160 ? a : b);
      p->drawLine(r.left(), y, r.right(), y);
    }
    p->setPen(QPen(QColor("#28d7c0"), 0));
    p->drawLine(0, r.top(), 0, r.bottom());
    p->setPen(QPen(QColor("#ff617e"), 0));
    p->drawLine(r.left(), 0, r.right(), 0);
    QRectF camera=portraitMode?QRectF(-225,-400,450,800):QRectF(-400,-225,800,450);
    p->setPen(QPen(QColor("#7c5cff"), 2, Qt::DashLine));
    p->setBrush(QColor(124, 92, 255, 12));
    p->drawRect(camera);
    p->setPen(QColor("#a999ff"));
    p->drawText(camera.topLeft() + QPointF(10, 22),
                "GAME CAMERA  16:9  •  1280 × 720");
  }
  void drawForeground(QPainter *p,const QRectF&) override {
    p->setRenderHint(QPainter::Antialiasing);
    for(auto *e:entities()){
      if(!e->isVisible())continue;
      QString type=e->data(TypeRole).toString();if(type!="Grid Map"&&type!="ASFO Territory"&&type!="Camera")continue;
      p->save();p->translate(e->pos());
      if(type=="Camera"){qreal w=qMax(1.0,e->data(WidthRole).toDouble()),h=qMax(1.0,e->data(HeightRole).toDouble());QRectF frame(-w/2,-h/2,w,h);p->setBrush(QColor(40,215,192,12));p->setPen(QPen(QColor("#28d7c0"),3,Qt::DashLine));p->drawRect(frame);p->setPen(QColor("#71f2df"));p->drawText(frame.adjusted(12,10,-12,-10),Qt::AlignTop|Qt::AlignLeft,QString("CAMERA • %1 • %2 × %3").arg(e->data(ShapeRole).toString()).arg(qRound(w)).arg(qRound(h)));p->restore();continue;}
      QColor fill=e->data(FillColorRole).value<QColor>(),stroke=e->data(StrokeColorRole).value<QColor>();
      if(type=="Grid Map"){
        int cell=e->data(CellSizeRole).toInt(),cols=e->data(ColumnsRole).toInt(),rows=e->data(RowsRole).toInt();QRectF map(0,0,cols*cell,rows*cell);
        p->fillRect(map,fill);QVariantMap colors=e->data(CellColorsRole).toMap();for(auto it=colors.begin();it!=colors.end();++it){QStringList xy=it.key().split(',');if(xy.size()==2)p->fillRect(QRectF(xy[0].toInt()*cell,xy[1].toInt()*cell,cell,cell),it.value().value<QColor>());}p->setPen(QPen(stroke,1));p->drawRect(map);
        if(e->data(GridVisibleRole).toBool()){for(int x=0;x<=cols;x++)p->drawLine(x*cell,0,x*cell,rows*cell);for(int y=0;y<=rows;y++)p->drawLine(0,y*cell,cols*cell,y*cell);}
        p->setPen(Qt::white);p->drawText(map.adjusted(12,12,-12,-12),Qt::AlignTop|Qt::AlignLeft,QString("GRID %1 × %2 • %3 px • %4 × %5 px").arg(cols).arg(rows).arg(cell).arg(cols*cell).arg(rows*cell));
      }else{
        qreal w=e->data(WidthRole).toDouble(),h=e->data(HeightRole).toDouble();QRectF area(-w/2,-h/2,w,h);QRadialGradient g(0,0,qMax(w,h)/2);g.setColorAt(0,fill.lighter(125));g.setColorAt(1,fill.darker(145));p->setBrush(g);p->setPen(QPen(stroke,3));QString shape=e->data(ShapeRole).toString();if(shape=="Square")p->drawRect(area);else if(shape=="Rounded Square")p->drawRoundedRect(area,40,40);else if(shape=="Hexagon"){QPolygonF hex;for(int i=0;i<6;i++){double a=i*M_PI/3;hex<<QPointF(std::cos(a)*w/2,std::sin(a)*h/2);}p->drawPolygon(hex);}else p->drawEllipse(area);p->setPen(Qt::white);p->drawText(area,Qt::AlignCenter,QString("ASFO %1\n%2 × %3 px").arg(shape).arg(w).arg(h));
      }p->restore();
    }
  }
  void wheelEvent(QWheelEvent *e) override {
    double f = e->angleDelta().y() > 0 ? 1.15 : 1 / 1.15;
    scale(f, f);
  }
  void mouseReleaseEvent(QMouseEvent *e) override {
    QGraphicsView::mouseReleaseEvent(e);
    if(snapEnabled&&e->button()==Qt::LeftButton){for(auto *item:world.selectedItems()){item->setPos(qRound(item->x()/snapSize)*snapSize,qRound(item->y()/snapSize)*snapSize);}world.update();viewport()->update();}
    if (changed)
      changed();
  }
};

class Window final : public QMainWindow {
  SceneView *view{};
  QTabWidget *tabs{};
  QPlainTextEdit *code{};
  QTextEdit *console{};
  QTreeWidget *tree{};
  QListWidget *scriptList{};
  QListWidget *projectObjects{};
  QDockWidget *projectDock{};
  QListWidget *tilePalette{};
  QSpinBox *tilePaletteCellSize{};
  QComboBox *orientationCombo{};
  QString gameOrientation="Landscape";
  int gamePixelWidth=1280,gamePixelHeight=720;
  QColor androidSystemBarColor=QColor("#0b0f18");
  QString gameName="Engine Plus Plus Game";
  QString gameIconPath;
  QMap<QString,QString> scripts;
  QMap<QString,QString> scriptTargets;
  QString currentScript="Main.cpp";
  QProcess *buildProcess{};
  QProcess *androidProcess{};
  QProcess *desktopProcess{};
  int androidStage=0;
  QString androidRoot,androidBuild,androidApk,androidFinalApk,androidSdk,androidNdk,androidCmake,androidNinja,androidDeploy,androidDeployJson;
  QProgressBar *buildProgress{};
  QComboBox *sceneCombo{};
  QStringList sceneNames{"Scene 1"};
  QString currentScene="Scene 1";
  QString projectDir;
  bool androidFullscreen=true;
  QFormLayout *form{};
  QString compiler;
  QString script =
      R"CPP(// This is real C++ and runs every frame in the game window.
// Available: scene.find("name"), entity.x/y, velocityX/Y, rotation,
// color, visible, scene.time, input.left/right/up/down/space.

void onStart(Scene& scene) {
    if (auto* player = scene.findType("Character")) {
        player->velocityX = 90.0f;
    }
}

void onUpdate(Scene& scene, float dt) {
    if (auto* player = scene.findType("Character")) {
        if (scene.input.left)  player->x -= 180.0f * dt;
        if (scene.input.right) player->x += 180.0f * dt;
        if (scene.input.up)    player->y -= 180.0f * dt;
        if (scene.input.down)  player->y += 180.0f * dt;

        // Code directly affects graphics:
        player->rotation += 35.0f * dt;
        player->color = QColor::fromHsvF(fmod(scene.time * .12, 1.0), .65, 1.0);
    }
}
)CPP";

public:
  Window() {
    setWindowTitle("Engine++ Studio v21 — Media, Haptics & Save Data");
    resize(1500, 900);
    setMinimumSize(1050, 650);
    setDockOptions(QMainWindow::AnimatedDocks|QMainWindow::AllowNestedDocks|QMainWindow::AllowTabbedDocks);
    compiler = QCoreApplication::applicationDirPath() + "/compiler/bin/g++.exe";
    if (!QFileInfo::exists(compiler))
      compiler = "C:/msys64/ucrt64/bin/g++.exe";
    projectDir=QDir(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).filePath("Engine Plus Plus Projects/Untitled");QDir().mkpath(projectDir+"/scripts");QDir().mkpath(projectDir+"/assets");
    setStyleSheet(css()+polishCss());
    toolbar();
    docks();
    center();
    scripts["Main.cpp"]=script;
    if(scriptList){scriptList->addItem("Main.cpp");scriptList->setCurrentRow(0);}
    extras();
    view->changed = [this] {
      hierarchy();
      inspector();
    };
    view->requestSave=[this]{save();};
    view->requestImage=[this](QGraphicsRectItem*r){chooseImage(r);};
    connect(&view->world, &QGraphicsScene::selectionChanged, this,
            &Window::inspector);
    hierarchy();
    buildProgress=new QProgressBar;buildProgress->setMaximumWidth(180);buildProgress->hide();statusBar()->addPermanentWidget(buildProgress);
    statusBar()->showMessage(
        "● GCC 16 C++20     ● Qt Graphics Runtime     Auto Optimize: O3 + LTO");
  }

private:
  QString lightCss() {
    return R"(*{font-family:'Segoe UI';font-size:13px;color:#172033}QMainWindow,QWidget{background:#f4f6fb}QToolBar{background:#fff;border-bottom:1px solid #ccd3e2;padding:8px}QToolButton,QPushButton,QComboBox,QLineEdit,QDoubleSpinBox{background:#fff;border:1px solid #c7cede;border-radius:7px;padding:8px 12px}QToolButton:checked{background:#684be2;color:white}QDockWidget::title{background:#eef1f8;padding:10px}QListWidget,QTreeWidget,QPlainTextEdit,QTextEdit{background:#fff;border:1px solid #cbd2e1;border-radius:6px;selection-background-color:#7960df;padding:5px}QTabBar::tab{background:#eef1f8;padding:11px 22px}QTabBar::tab:selected{background:#fff;border-bottom:2px solid #684be2}QStatusBar{background:#fff;border-top:1px solid #ccd3e2})";
  }
  QString polishCss(){return R"(QMenu{color:#f2f5ff;background:#151b2b;border:1px solid #465477;border-radius:12px;padding:8px}QMenu::item{color:#f2f5ff;padding:10px 32px 10px 14px;border-radius:8px}QMenu::item:selected{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #7657eb,stop:1 #4f83e8);color:#ffffff}QMenu::item:disabled{color:#727e9b}QMenu::separator{height:1px;background:#303952;margin:7px}QScrollBar:vertical{background:#0b101c;width:12px;margin:2px}QScrollBar::handle:vertical{background:#465474;border-radius:6px;min-height:34px}QScrollBar::handle:vertical:hover{background:#8268ed}QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0}QToolBar{min-height:50px}QStatusBar{min-height:29px}QDockWidget{border:1px solid #2d3855}QDockWidget::title{font-weight:750;letter-spacing:.7px;background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #151d31,stop:1 #101725)}QTabWidget::pane{border-top:1px solid #2f3955}QComboBox QAbstractItemView{color:#f2f5ff;background:#111827;border:1px solid #465477;border-radius:10px;padding:6px;selection-color:#ffffff;selection-background-color:#7657eb}QToolTip{color:#ffffff;background:#202a40;border:1px solid #6b79a0;padding:7px}QToolButton#snap:checked{background:#27d6b8;color:#071511;border-color:#78f2dd;font-weight:800}QPushButton#addNode{color:#ffffff;font-size:13px;background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #7657eb,stop:1 #4e70db)}QLineEdit:focus,QSpinBox:focus,QDoubleSpinBox:focus,QComboBox:focus{border:1px solid #9b84ff;background:#171f33}QCheckBox{spacing:9px}QCheckBox::indicator{width:18px;height:18px;border:1px solid #5d6b91;border-radius:5px;background:#101725}QCheckBox::indicator:checked{background:#7657eb;border-color:#aa98ff})";}
  void extras() {
    auto *b = findChild<QToolBar *>();
    auto makeMenu=[b](const QString&text){auto*button=new QToolButton;button->setText(text);button->setPopupMode(QToolButton::InstantPopup);button->setMenu(new QMenu(button));button->setObjectName("menuButton");b->addWidget(button);return button->menu();};
    auto *viewMenu=makeMenu("VIEW  ▾"),*scriptMenu=makeMenu("SCRIPTS  ▾"),*buildMenu=makeMenu("BUILD  ▾");
    auto *bottomPanel=projectDock->toggleViewAction();bottomPanel->setText("Bottom panel");bottomPanel->setToolTip("Show or hide the compact scripts panel");viewMenu->addAction(bottomPanel);
    auto *theme = viewMenu->addAction("Light Theme");
    theme->setCheckable(true);
    connect(theme, &QAction::toggled, this, [this, theme](bool on) {
      setStyleSheet((on ? lightCss() : css())+polishCss());
      theme->setText(on ? "☾ DARK THEME" : "☼ LIGHT THEME");
    });
    auto *create = scriptMenu->addAction("Create Character Script");
    connect(create, &QAction::triggered, this, [this] {
      auto s = view->world.selectedItems();
      if (s.isEmpty()) {
        QMessageBox::information(this, "Character script",
                                 "Select a Character first.");
        return;
      }
      auto *r = qgraphicsitem_cast<QGraphicsRectItem *>(s.first());
      if (!r || r->data(TypeRole) != "Character") {
        QMessageBox::information(this, "Character script",
                                 "Selected object is not a Character.");
        return;
      }
      QString name = r->data(NameRole).toString();
      r->setData(ScriptRole, "Custom C++ Movement");
      code->setPlainText(QString(R"CPP(// Editable movement script for: %1
void onStart(Scene& scene) {
    if (auto* character = scene.find("%1")) {
        character->velocityX = 0.0f;
        character->velocityY = 0.0f;
    }
}
void onUpdate(Scene& scene, float dt) {
    auto* character = scene.find("%1");
    if (!character) return;
    float speed = 220.0f; // Change speed here
    if (scene.input.left) character->x -= speed * dt;
    if (scene.input.right) character->x += speed * dt;
    if (scene.input.up) character->y -= speed * dt;
    if (scene.input.down) character->y += speed * dt;
    if (scene.input.space) character->rotation += 180.0f * dt;
}
)CPP")
                             .arg(name));
      tabs->setCurrentIndex(1);
      hierarchy();
    });
    auto *android=buildMenu->addAction("Export Android APK");
    connect(android,&QAction::triggered,this,&Window::exportAndroid);
    auto *windowsExport=buildMenu->addAction("Export Windows EXE");
    connect(windowsExport,&QAction::triggered,this,&Window::exportWindows);
    auto *portableExport=buildMenu->addAction("Export Linux / macOS / Windows Project");
    connect(portableExport,&QAction::triggered,this,&Window::exportPortableProject);
    auto *fullscreen=buildMenu->addAction("Android Fullscreen");fullscreen->setCheckable(true);fullscreen->setChecked(androidFullscreen);fullscreen->setToolTip("Launch the Android game without desktop-style window borders");connect(fullscreen,&QAction::toggled,this,[this](bool on){androidFullscreen=on;statusBar()->showMessage(on?"Android fullscreen enabled":"Android fullscreen disabled",3000);});
    auto *systemBars=buildMenu->addAction("Android System Bars Color...");connect(systemBars,&QAction::triggered,this,[this]{if(androidFullscreen){QMessageBox::information(this,"Android system bars","Disable Android Fullscreen first. System bars are hidden in fullscreen mode.");return;}QColor chosen=QColorDialog::getColor(androidSystemBarColor,this,"Android status/navigation bars color");if(chosen.isValid()){androidSystemBarColor=chosen;statusBar()->showMessage("Android system bars color: "+chosen.name(),3500);}});
    auto *gameSettings=buildMenu->addAction("Game Name && Icon...");connect(gameSettings,&QAction::triggered,this,[this]{QDialog d(this);d.setWindowTitle("Game identity");d.resize(520,210);auto*f=new QFormLayout(&d);auto*name=new QLineEdit(gameName);auto*icon=new QPushButton(gameIconPath.isEmpty()?"Choose PNG / ICO":QFileInfo(gameIconPath).fileName());auto*buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);f->addRow("Game name",name);f->addRow("Windows / Android icon",icon);f->addRow(buttons);connect(icon,&QPushButton::clicked,&d,[this,icon]{QString file=QFileDialog::getOpenFileName(this,"Game icon",{},"Icons (*.png *.ico *.jpg *.webp)");if(!file.isEmpty()){QString target=projectDir+"/assets/game_icon."+QFileInfo(file).suffix().toLower();QFile::remove(target);if(QFileInfo(file).absoluteFilePath()==QFileInfo(target).absoluteFilePath()||QFile::copy(file,target)){gameIconPath=target;icon->setText(QFileInfo(target).fileName());}}});connect(buttons,&QDialogButtonBox::accepted,&d,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,&d,&QDialog::reject);if(d.exec()==QDialog::Accepted){gameName=name->text().trimmed().isEmpty()?"Engine Plus Plus Game":name->text().trimmed();save();statusBar()->showMessage("Game identity updated: "+gameName,4000);}});
    auto *objectScript=scriptMenu->addAction("Create Object Script");
    connect(objectScript,&QAction::triggered,this,[this]{auto selected=view->world.selectedItems();if(selected.isEmpty()){QMessageBox::information(this,"Object script","Select any scene object first.");return;}auto*r=qgraphicsitem_cast<QGraphicsRectItem*>(selected.first());if(!r)return;QString object=r->data(NameRole).toString(),safe=object;safe.replace(QRegularExpression("[^A-Za-z0-9_]"),"_");QString file=safe+".cpp";scripts[currentScript]=code->toPlainText();scripts[file]=QString("// Component script attached to %1\nvoid update_%2(Scene& scene, Entity& self, float dt) {\n    Q_UNUSED(scene);\n    // Example: self.rotation += 45.0f * dt;\n    // Camera: self.x, self.y or self.cameraFollow\n    // Light: self.color, self.width, self.height\n}\n").arg(object,safe);scriptTargets[file]=object;r->setData(ScriptRole,file);if(!scriptList->findItems(file,Qt::MatchExactly).size())scriptList->addItem(file);scriptList->setCurrentRow(scriptList->row(scriptList->findItems(file,Qt::MatchExactly).first()));hierarchy();});
  }
  QString css() {
    return R"(*{font-family:'Segoe UI';font-size:13px;color:#edf1ff}QMainWindow,QWidget{background:#090d16}QToolBar{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #151b2b,stop:1 #0f1421);border:0;border-bottom:1px solid #303a56;padding:10px;spacing:8px}QToolButton,QPushButton{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #242d44,stop:1 #171e30);border:1px solid #3a4665;border-radius:9px;padding:8px 13px}QComboBox,QLineEdit,QDoubleSpinBox,QSpinBox{background:#141b2b;border:1px solid #36415f;border-radius:8px;padding:8px 11px}QToolButton:hover,QPushButton:hover{border-color:#8c75ff;background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #343f60,stop:1 #222a43)}QToolButton:pressed,QPushButton:pressed{background:#111725;border-color:#5f4fc2;padding-top:9px;padding-bottom:7px}QToolButton:checked{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #8064f2,stop:1 #5739c5);border-color:#a997ff}QToolButton#run{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #8065f3,stop:1 #5637c7);border-color:#a997ff;font-weight:800;padding-left:22px;padding-right:22px}QToolButton#menuButton{min-width:82px;font-weight:700}QDockWidget{font-weight:600;color:#aeb8d5}QDockWidget::title{background:#101624;padding:11px;border-bottom:1px solid #2c3651}QListWidget,QTreeWidget,QPlainTextEdit,QTextEdit{background:#0c111d;border:1px solid #293550;border-radius:9px;selection-background-color:#684be2;padding:6px}QListWidget::item,QTreeWidget::item{padding:8px;border-radius:6px}QListWidget::item:hover,QTreeWidget::item:hover{background:#1b2438}QTabWidget::pane{border:0}QTabBar::tab{background:#101624;color:#929fbd;padding:12px 23px;border-bottom:2px solid transparent}QTabBar::tab:hover{color:#d9dfff;background:#171f32}QTabBar::tab:selected{color:#fff;border-bottom:2px solid #8c74ff;background:#1a2236}QStatusBar{background:#0f1522;border-top:1px solid #29344e;color:#a8b3ce}QSplitter::handle{background:#303a55})";
  }
  QString newSceneEntry() const {return QString::fromUtf8("＋  НОВА СЦЕНА...");}
  void createSceneDialog(){
    bool ok=false;QString name=QInputDialog::getText(this,QString::fromUtf8("Нова сцена"),QString::fromUtf8("Назва сцени:"),QLineEdit::Normal,"Scene "+QString::number(sceneNames.size()+1),&ok).trimmed();
    if(!ok||name.isEmpty()||sceneNames.contains(name)){QSignalBlocker guard(sceneCombo);sceneCombo->setCurrentText(currentScene);return;}
    sceneNames<<name;sceneCombo->insertItem(qMax(0,sceneCombo->count()-1),name);sceneCombo->setCurrentText(name);
  }
  void toolbar() {
    auto *b = addToolBar("Main");
    b->setMovable(false);
    auto *logo = new QLabel("  E++  ENGINE PLUS PLUS   ");
    logo->setStyleSheet("font-size:15px;font-weight:700");
    b->addWidget(logo);
    auto *scene = b->addAction("▦ SCENE");
    auto *cpp = b->addAction("</> C++ SCRIPT");
    sceneCombo=new QComboBox;sceneCombo->addItems(sceneNames);sceneCombo->addItem(newSceneEntry());sceneCombo->setMinimumWidth(185);sceneCombo->setToolTip(QString::fromUtf8("Вибір сцени. Команда створення нової сцени завжди внизу."));b->addWidget(sceneCombo);auto*snap=b->addAction(QString::fromUtf8("🧲 SNAP 32"));snap->setCheckable(true);snap->setChecked(true);snap->setToolTip(QString::fromUtf8("Примагнічувати об'єкти до сітки 32 px"));b->widgetForAction(snap)->setObjectName("snap");auto *projectButton=new QToolButton;projectButton->setText("PROJECT  ▾");projectButton->setPopupMode(QToolButton::InstantPopup);auto *projectMenu=new QMenu(projectButton);projectButton->setMenu(projectMenu);b->addWidget(projectButton);auto*newProject=projectMenu->addAction("Create Project");auto*load=projectMenu->addAction("Open Project");
    orientationCombo=new QComboBox;orientationCombo->addItems({"1280 × 720  •  16:9","1920 × 1080  •  16:9","1600 × 900  •  16:9","2560 × 1440  •  16:9","720 × 1280  •  9:16","1080 × 1920  •  9:16","900 × 1600  •  9:16","1080 × 1080  •  1:1","CUSTOM RESOLUTION..."});orientationCombo->setMinimumWidth(195);orientationCombo->setToolTip("Game resolution and aspect ratio");b->addWidget(orientationCombo);connect(orientationCombo,&QComboBox::currentIndexChanged,this,[this](int index){int w=gamePixelWidth,h=gamePixelHeight;if(index==orientationCombo->count()-1){QDialog dialog(this);dialog.setWindowTitle("Custom game resolution");auto*formLayout=new QFormLayout(&dialog);auto*widthBox=new QSpinBox,*heightBox=new QSpinBox;for(auto*box:{widthBox,heightBox})box->setRange(240,8192);widthBox->setValue(gamePixelWidth);heightBox->setValue(gamePixelHeight);formLayout->addRow("Width (pixels)",widthBox);formLayout->addRow("Height (pixels)",heightBox);auto*buttons=new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);formLayout->addRow(buttons);connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);if(dialog.exec()!=QDialog::Accepted)return;w=widthBox->value();h=heightBox->value();orientationCombo->setItemText(index,QString("CUSTOM  %1 × %2").arg(w).arg(h));}else{QRegularExpressionMatch match=QRegularExpression("(\\d+)\\s*×\\s*(\\d+)").match(orientationCombo->itemText(index));if(match.hasMatch()){w=match.captured(1).toInt();h=match.captured(2).toInt();}}gamePixelWidth=w;gamePixelHeight=h;gameOrientation=h>w?"Portrait":(h==w?"Square":"Landscape");if(view){view->portraitMode=h>w;double cameraW,cameraH;if(w>=h){cameraW=800.0;cameraH=800.0*h/w;}else{cameraH=800.0;cameraW=800.0*w/h;}for(auto*camera:view->entities())if(camera->data(TypeRole)=="Camera"&&camera->data(ShapeRole).toString()=="Follow Game"){camera->setData(WidthRole,cameraW);camera->setData(HeightRole,cameraH);}view->world.update();view->fitInView(QRectF(-cameraW*.62,-cameraH*.62,cameraW*1.24,cameraH*1.24),Qt::KeepAspectRatio);}statusBar()->showMessage(QString("Game resolution: %1 × %2").arg(w).arg(h),3000);});
    scene->setCheckable(1);
    cpp->setCheckable(1);
    scene->setChecked(1);
    auto *g = new QActionGroup(this);
    g->addAction(scene);
    g->addAction(cpp);
    g->setExclusive(1);
    auto *sp = new QWidget;
    sp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    b->addWidget(sp);
    auto *opt = b->addAction("⚡ AUTO OPTIMIZE");
    opt->setCheckable(1);
    opt->setChecked(1);
    auto *save = b->addAction("SAVE");
    auto *run = b->addAction("▶ RUN GAME");run->setShortcut(QKeySequence("F6"));
    b->widgetForAction(run)->setObjectName("run");
    connect(scene, &QAction::triggered, this,
            [this] { tabs->setCurrentIndex(0); });
    connect(cpp, &QAction::triggered, this,
            [this] { tabs->setCurrentIndex(1); });
    connect(save, &QAction::triggered, this, &Window::save);
    connect(newProject,&QAction::triggered,this,&Window::createProject);
    connect(load,&QAction::triggered,this,&Window::chooseLoadProject);
    connect(snap,&QAction::toggled,this,[this](bool on){if(view)view->snapEnabled=on;statusBar()->showMessage(on?QString::fromUtf8("Примагнічування 32 px увімкнено"):QString::fromUtf8("Примагнічування вимкнено"),2500);});
    connect(sceneCombo,&QComboBox::currentTextChanged,this,[this](const QString&s){if(s==newSceneEntry()){QTimer::singleShot(0,this,&Window::createSceneDialog);return;}currentScene=s;for(auto*e:view->entities())e->setVisible(e->data(SceneRole).toString()==s&&e->data(ObjectVisibleRole).toBool());hierarchy();view->viewport()->update();});
    connect(run, &QAction::triggered, this, &Window::run);
  }
  void showNodePicker(){
    QDialog dialog(this);dialog.setWindowTitle("Add Node");dialog.resize(860,650);dialog.setModal(true);dialog.setStyleSheet("QDialog{background:#070b13}QFrame#nodeCard{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #151c2d,stop:1 #0e1421);border:1px solid #35415f;border-radius:22px}QLabel#nodeTitle{font-size:26px;font-weight:850;color:#ffffff}QLabel#nodeCount{background:#6b50dc;color:white;border:1px solid #9b87ff;border-radius:13px;padding:5px 11px;font-weight:800}QLineEdit{background:#0a101d;color:#f5f7ff;border:1px solid #3b4868;border-radius:11px;padding:12px 15px;font-size:14px}QPushButton[class=\"node\"]{color:#edf2ff;background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #202b43,stop:1 #151d30);border:1px solid #3a496d;border-radius:14px;padding:13px;text-align:left;font-size:14px;font-weight:650}QPushButton[class=\"node\"]:hover{background:qlineargradient(x1:0,y1:0,x2:0,y2:1,stop:0 #38476d,stop:1 #283554);border-color:#957fff;color:#ffffff}QPushButton[class=\"node\"]:pressed{background:#151b2b;border-color:#6951d1;padding-top:14px;padding-bottom:12px}QPushButton#close{color:#aeb9d5;background:#1a2235;border:1px solid #35415f;border-radius:10px;font-size:20px;padding:6px}QPushButton#close:hover{color:white;border-color:#ff6b85;background:#3a1e2b}");
    auto *outer=new QVBoxLayout(&dialog);outer->setContentsMargins(18,18,18,18);auto *card=new QFrame;card->setObjectName("nodeCard");outer->addWidget(card);auto *layout=new QVBoxLayout(card);layout->setContentsMargins(26,22,26,26);layout->setSpacing(15);auto *head=new QHBoxLayout;auto *title=new QLabel("Add a Node");title->setObjectName("nodeTitle");auto *countBadge=new QLabel;countBadge->setObjectName("nodeCount");auto *close=new QPushButton("×");close->setObjectName("close");close->setFixedSize(40,40);head->addWidget(title);head->addWidget(countBadge);head->addStretch();head->addWidget(close);layout->addLayout(head);auto *hint=new QLabel("Choose an object for the current scene • "+currentScene);hint->setStyleSheet("color:#9da9c8;font-size:13px");layout->addWidget(hint);auto *search=new QLineEdit;search->setPlaceholderText("Search all nodes...");layout->addWidget(search);
    auto *scroll=new QScrollArea;scroll->setWidgetResizable(true);scroll->setFrameShape(QFrame::NoFrame);auto *page=new QWidget;auto *grid=new QGridLayout(page);grid->setSpacing(10);scroll->setWidget(page);layout->addWidget(scroll,1);const QList<QPair<QString,QString>> nodes={{"Background Image","Full-screen image • no hitbox"},{"Color Block","Solid or softly blurred color"},{"Physics Block","Dynamic block with gravity"},{"Camera","Viewport, aspect and culling"},{"Character","Player object with gravity + touch"},{"Platform","Solid platform with hitbox"},{"Solid Ground","Static solid ground"},{"Sprite","Image-based scene object"},{"Grid Map","Configurable cells and paint"},{"Tile Map","Godot-style tileset painting"},{"ASFO Territory","Circular or polygon territory"},{"Button","Clickable scene transition"},{"Button Container","Several arranged buttons"},{"Dropdown","Selectable options list"},{"Toggle","On / Off switch"},{"Slider","Interactive numeric slider"},{"Progress Bar","Visual progress value"},{"Check Box","Independent checked state"},{"Radio Button","Exclusive option in a group"},{"Scroll View","Clipped scrollable content"},{"Theme Switch","Change many target objects"},{"Text","Custom text and font"},{"Panel","Reusable UI panel"},{"Glass Panel","Translucent glass-style panel"},{"UI Container","Layout container"},{"Light","Radial lighting"},{"Particle","Particle visual effect"},{"Collider","Invisible collision area"},{"Shader","Visual shader object"},{"Scene Portal","Transition to another scene"},{"Video Player","MP4, WebM and video playback"},{"Audio Source","MP3, WAV, OGG music and sounds"},{"Vibration","Android haptic feedback block"},{"Save Data","Persistent game progress block"}};
    countBadge->setText(QString("%1 NODES").arg(nodes.size()));QList<QPushButton*> buttons;int row=0,col=0;for(const auto &node:nodes){auto *button=new QPushButton(node.first+"\n"+node.second);button->setProperty("class","node");button->setProperty("nodeType",node.first);button->setMinimumSize(235,72);grid->addWidget(button,row,col);buttons<<button;if(++col==3){col=0;row++;}connect(button,&QPushButton::clicked,&dialog,[this,&dialog,node]{dialog.accept();view->add(node.first);auto all=view->entities();if(!all.isEmpty()){auto*r=all.last();r->setData(SceneRole,currentScene);r->setVisible(true);r->setSelected(true);}hierarchy();inspector();});}grid->setRowStretch(row+1,1);connect(search,&QLineEdit::textChanged,&dialog,[buttons,countBadge](const QString &text){int visible=0;for(auto*b:buttons){bool match=text.trimmed().isEmpty()||b->text().contains(text,Qt::CaseInsensitive);b->setVisible(match);if(match)visible++;}countBadge->setText(QString("%1 / %2 NODES").arg(visible).arg(buttons.size()));});connect(close,&QPushButton::clicked,&dialog,&QDialog::reject);dialog.exec();
  }
  void docks() {
    auto *l = new QDockWidget("SCENE TREE", this);
    l->setFeatures(QDockWidget::NoDockWidgetFeatures);
    auto *p = new QWidget;
    auto *v = new QVBoxLayout(p);
    auto *assets = new QListWidget;
    assets->addItems({"◆  Character", "▰  Platform", "●  Sprite", "▧  Background Image", "☀  Light",
                      "◉  Camera", "✦  Particle", "◈  Shader", "□  Collider",
                      "T  Text", "▣  Button", "▤  UI Container", "▶  Video Player",
                      "♫  Audio Source", "▦  Tile Map", "▧  Scene Portal",
                      "#  Grid Map", "O  ASFO Territory", "=  Solid Ground"});
    assets->hide();auto *addNode=new QPushButton("＋  ADD NODE");addNode->setObjectName("addNode");addNode->setMinimumHeight(42);addNode->setStyleSheet("QPushButton{background:#684be2;color:white;border:1px solid #957fff;border-radius:10px;font-weight:800;padding:10px}QPushButton:hover{background:#7b61ef}");v->addWidget(addNode);
    auto *lab = new QLabel("SCENE HIERARCHY");
    lab->setStyleSheet("color:#8e9ab8;margin-top:12px");
    v->addWidget(lab);
    tree = new QTreeWidget;
    tree->setHeaderHidden(1);
    v->addWidget(tree, 2);
    l->setWidget(p);
    addDockWidget(Qt::LeftDockWidgetArea, l);
    l->setMinimumWidth(245);
    connect(addNode,&QPushButton::clicked,this,&Window::showNodePicker);
    connect(assets, &QListWidget::itemDoubleClicked, this,
            [this](QListWidgetItem *i) { view->add(i->text().mid(3));auto all=view->entities();if(!all.isEmpty()){all.last()->setData(SceneRole,currentScene);all.last()->setVisible(true);}hierarchy(); });
    connect(tree, &QTreeWidget::itemClicked, this, [this](QTreeWidgetItem *i) {
      int n = i->data(0, Qt::UserRole).toInt();
      auto a = view->entities();
      if (n >= 0 && n < a.size()) {
        view->world.clearSelection();
        a[n]->setSelected(1);
        view->centerOn(a[n]);
      }
    });
    auto *r = new QDockWidget("COMPONENT INSPECTOR", this);
    r->setFeatures(QDockWidget::NoDockWidgetFeatures);
    auto *w = new QWidget;
    form = new QFormLayout(w);
    form->setContentsMargins(14, 14, 14, 14);
    auto *inspectorScroll=new QScrollArea;inspectorScroll->setWidgetResizable(true);inspectorScroll->setFrameShape(QFrame::NoFrame);inspectorScroll->setWidget(w);r->setWidget(inspectorScroll);
    addDockWidget(Qt::RightDockWidgetArea, r);
    r->setMinimumWidth(285);
    projectDock=new QDockWidget("PROJECT • SCRIPTS",this);projectDock->setAllowedAreas(Qt::BottomDockWidgetArea);projectDock->setFeatures(QDockWidget::DockWidgetClosable|QDockWidget::DockWidgetMovable|QDockWidget::DockWidgetFloatable);
    auto *projectTabs=new QTabWidget;
    projectObjects=nullptr;
    auto *scriptsPage=new QWidget;auto *sv=new QVBoxLayout(scriptsPage);sv->setContentsMargins(8,6,8,8);sv->setSpacing(6);scriptList=new QListWidget;scriptList->setFlow(QListView::LeftToRight);scriptList->setWrapping(false);scriptList->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);scriptList->setMaximumHeight(54);auto *newScript=new QPushButton("+ New C++ Script");newScript->setMaximumWidth(180);sv->addWidget(scriptList);sv->addWidget(newScript,0,Qt::AlignLeft);projectTabs->addTab(scriptsPage,"SCRIPTS");
    auto *tilePage=new QWidget;auto *tv=new QHBoxLayout(tilePage);tv->setContentsMargins(8,6,8,8);auto *tileTools=new QVBoxLayout;auto *chooseTileset=new QPushButton("Choose Tileset Image");tilePaletteCellSize=new QSpinBox;tilePaletteCellSize->setRange(4,512);tilePaletteCellSize->setValue(32);tilePaletteCellSize->setSuffix(" px");tileTools->addWidget(chooseTileset);tileTools->addWidget(new QLabel("Tile size"));tileTools->addWidget(tilePaletteCellSize);tileTools->addStretch();tilePalette=new QListWidget;tilePalette->setViewMode(QListView::IconMode);tilePalette->setIconSize(QSize(56,56));tilePalette->setGridSize(QSize(72,76));tilePalette->setResizeMode(QListView::Adjust);tilePalette->setMovement(QListView::Static);tilePalette->setToolTip("Select a tile, then click cells on the selected Tile Map");tv->addLayout(tileTools);tv->addWidget(tilePalette,1);projectTabs->addTab(tilePage,"TILE PALETTE");
    projectDock->setWidget(projectTabs);addDockWidget(Qt::BottomDockWidgetArea,projectDock);projectDock->setMinimumHeight(100);projectDock->setMaximumHeight(270);resizeDocks({projectDock},{150},Qt::Vertical);
    connect(chooseTileset,&QPushButton::clicked,this,[this]{auto selected=view->world.selectedItems();if(selected.isEmpty()||selected.first()->data(TypeRole)!="Tile Map"){QMessageBox::information(this,"Tile Palette","Select a Tile Map in the scene first.");return;}auto*r=qgraphicsitem_cast<QGraphicsRectItem*>(selected.first());chooseImage(r);if(r){r->setData(CellSizeRole,tilePaletteCellSize->value());r->setData(CellPaintModeRole,true);rebuildTilePalette(r);}});
    connect(tilePalette,&QListWidget::itemClicked,this,[this](QListWidgetItem*item){auto selected=view->world.selectedItems();if(selected.isEmpty()||selected.first()->data(TypeRole)!="Tile Map")return;selected.first()->setData(TileIndexRole,item->data(Qt::UserRole));selected.first()->setData(CellPaintModeRole,true);statusBar()->showMessage(QString("Tile %1 selected - paint on the Tile Map").arg(item->data(Qt::UserRole).toInt()),3000);});
    connect(tilePaletteCellSize,qOverload<int>(&QSpinBox::valueChanged),this,[this](int value){auto selected=view->world.selectedItems();if(selected.isEmpty()||selected.first()->data(TypeRole)!="Tile Map")return;auto*r=qgraphicsitem_cast<QGraphicsRectItem*>(selected.first());int w=r->data(ColumnsRole).toInt()*value,h=r->data(RowsRole).toInt()*value;r->setData(CellSizeRole,value);r->setData(WidthRole,w);r->setData(HeightRole,h);r->setRect(0,0,w,h);rebuildTilePalette(r);view->viewport()->update();});
    connect(newScript,&QPushButton::clicked,this,[this]{bool ok=false;QString name=QInputDialog::getText(this,"New script","Script name:",QLineEdit::Normal,"Script"+QString::number(scripts.size()+1)+".cpp",&ok);if(!ok||name.trimmed().isEmpty())return;if(!name.endsWith(".cpp"))name+=".cpp";scripts[currentScript]=code->toPlainText();scripts[name]="// "+name+"\n// Add helper functions here. Main.cpp owns onStart/onUpdate.\n";scriptList->addItem(name);scriptList->setCurrentRow(scriptList->count()-1);});
    connect(scriptList,&QListWidget::currentTextChanged,this,[this](const QString&name){if(name.isEmpty()||name==currentScript)return;scripts[currentScript]=code->toPlainText();currentScript=name;QSignalBlocker guard(code);code->setPlainText(scripts.value(name));tabs->setCurrentIndex(1);});
  }
  void center() {
    tabs = new QTabWidget;
    tabs->setDocumentMode(true);tabs->setMovable(true);tabs->tabBar()->setExpanding(false);
    view = new SceneView;
    tabs->addTab(view, "SCENE VIEWPORT");
    auto *page = new QWidget;
    auto *s = new QSplitter(Qt::Vertical);
    code = new QPlainTextEdit(script);
    code->setFont(QFont("Cascadia Mono", 11));
    code->setLineWrapMode(QPlainTextEdit::NoWrap);
    console = new QTextEdit;
    console->setReadOnly(1);
    console->setFont(QFont("Cascadia Mono", 10));
    console->setText("Runtime ready. Your C++ onUpdate() controls the rendered "
                     "game.\nCompiler: " +
                     compiler);
    s->addWidget(code);
    s->addWidget(console);
    s->setStretchFactor(0, 4);
    s->setStretchFactor(1, 1);
    auto *v = new QVBoxLayout(page);
    v->setContentsMargins(0, 0, 0, 0);
    v->addWidget(s);
    tabs->addTab(page, "C++ GAME SCRIPT");
    setCentralWidget(tabs);
    connect(code, &QPlainTextEdit::textChanged, this, &Window::syncInspectorFromCode);
  }
  void hierarchy() {
    tree->clear();
    if(projectObjects)projectObjects->clear();
    auto *root=new QTreeWidgetItem({"▾  "+currentScene});root->setData(0,Qt::UserRole,-1);root->setForeground(0,QColor("#b8c2e0"));QFont rootFont=root->font(0);rootFont.setBold(true);root->setFont(0,rootFont);tree->addTopLevelItem(root);
    int n = 0,index=0;
    for (auto *r : view->entities()) {
      if(r->data(SceneRole).toString()!=currentScene){index++;continue;}
      auto *i = new QTreeWidgetItem({QString("◆  %1   [%2]")
                                         .arg(r->data(NameRole).toString(),
                                              r->data(ScriptRole).toString())});
      i->setData(0, Qt::UserRole, index++);n++;
      root->addChild(i);
      if(projectObjects)projectObjects->addItem(r->data(TypeRole).toString()+"  •  "+r->data(NameRole).toString());
    }
    root->setExpanded(true);statusBar()->showMessage(
        QString("● GCC C++20     ● Scene: %1 objects     ⚡ O3 + LTO").arg(n));
  }
  void clearForm() {
    while (form->rowCount())
      form->removeRow(0);
  }
  void syncCodeFromInspector(QGraphicsRectItem *r) {
    QString block = QString("// BEGIN ENGINE PROPERTIES [%1]\n"
                            "// These values are synchronized with Inspector.\n"
                            "constexpr int ENGINE_cellSize = %2;\n"
                            "constexpr int ENGINE_columns = %3;\n"
                            "constexpr int ENGINE_rows = %4;\n"
                            "constexpr bool ENGINE_gridVisible = %5;\n"
                            "constexpr float ENGINE_gravity = %6f;\n"
                            "constexpr bool ENGINE_lockRotation = %7;\n"
                            "constexpr bool ENGINE_cameraAdaptive = %8;\n"
                            "constexpr int ENGINE_cullMargin = %9;\n"
                            "// END ENGINE PROPERTIES\n")
      .arg(r->data(NameRole).toString()).arg(r->data(CellSizeRole).toInt())
      .arg(r->data(ColumnsRole).toInt()).arg(r->data(RowsRole).toInt())
      .arg(r->data(GridVisibleRole).toBool()?"true":"false")
      .arg(QString::number(r->data(GravityRole).toDouble(),'f',1)).arg(r->data(LockRotationRole).toBool()?"true":"false")
      .arg(r->data(CameraAdaptiveRole).toBool()?"true":"false").arg(r->data(CullMarginRole).toInt());
    QString text=code->toPlainText();
    QRegularExpression marker(R"(// BEGIN ENGINE PROPERTIES \[[^\n]*\][\s\S]*?// END ENGINE PROPERTIES\n?)");
    QSignalBlocker guard(code);
    if(text.contains(marker)) text.replace(marker,block); else text=block+"\n"+text;
    code->setPlainText(text);
  }
  void syncInspectorFromCode() {
    auto selected=view->world.selectedItems(); if(selected.isEmpty()) return;
    auto *r=qgraphicsitem_cast<QGraphicsRectItem*>(selected.first()); if(!r) return;
    QString t=code->toPlainText();
    auto number=[&](QString key,double fallback){QRegularExpression re("ENGINE_"+key+R"(\s*=\s*([-+]?[0-9]*\.?[0-9]+))");auto m=re.match(t);return m.hasMatch()?m.captured(1).toDouble():fallback;};
    auto flag=[&](QString key,bool fallback){QRegularExpression re("ENGINE_"+key+R"(\s*=\s*(true|false))");auto m=re.match(t);return m.hasMatch()?m.captured(1)=="true":fallback;};
    r->setData(CellSizeRole,(int)number("cellSize",r->data(CellSizeRole).toInt()));
    r->setData(ColumnsRole,(int)number("columns",r->data(ColumnsRole).toInt()));
    r->setData(RowsRole,(int)number("rows",r->data(RowsRole).toInt()));
    r->setData(GridVisibleRole,flag("gridVisible",r->data(GridVisibleRole).toBool()));
    r->setData(GravityRole,number("gravity",r->data(GravityRole).toDouble()));
    r->setData(LockRotationRole,flag("lockRotation",r->data(LockRotationRole).toBool()));
    r->setData(CameraAdaptiveRole,flag("cameraAdaptive",r->data(CameraAdaptiveRole).toBool()));
    r->setData(CullMarginRole,(int)number("cullMargin",r->data(CullMarginRole).toInt()));
    view->viewport()->update();
  }
  void chooseImage(QGraphicsRectItem *r){
    if(!r)return;QString type=r->data(TypeRole).toString();QString title="Object image",filter="Images (*.png *.jpg *.jpeg *.bmp *.webp *.gif)";if(type=="Video Player"){title="Choose video";filter="Videos (*.mp4 *.m4v *.webm *.avi *.mov *.mkv);;All files (*.*)";}else if(type=="Audio Source"){title="Choose music or sound";filter="Audio (*.mp3 *.wav *.ogg *.m4a *.aac *.flac);;All files (*.*)";}QString file=QFileDialog::getOpenFileName(this,title,{},filter);if(file.isEmpty())return;
    QString assetsDir=projectDir+"/assets";QDir().mkpath(assetsDir);QString target=assetsDir+"/"+QFileInfo(file).fileName();
    if(QFileInfo(file).absoluteFilePath()!=QFileInfo(target).absoluteFilePath()){QFile::remove(target);if(!QFile::copy(file,target)){QMessageBox::warning(this,"Asset","Could not copy the file into the project assets folder.");return;}}
    r->setData(AssetRole,target);QPixmap px(target);if(!px.isNull()){QSize targetSize=qMax(1,qRound(r->rect().width()))>0?r->rect().size().toSize():QSize(60,60);QPixmap preview=px.scaled(targetSize,Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);QBrush brush(preview);brush.setTransform(QTransform::fromTranslate(r->rect().left(),r->rect().top()));r->setBrush(brush);}view->viewport()->update();
  }
  void chooseAlternateImage(QGraphicsRectItem *r){
    if(!r)return;QString file=QFileDialog::getOpenFileName(this,"Dark theme image",{},"Images (*.png *.jpg *.jpeg *.bmp *.webp *.gif)");if(file.isEmpty())return;QString assetsDir=projectDir+"/assets";QDir().mkpath(assetsDir);QString target=assetsDir+"/dark_"+QFileInfo(file).fileName();if(QFileInfo(file).absoluteFilePath()!=QFileInfo(target).absoluteFilePath()){QFile::remove(target);if(!QFile::copy(file,target)){QMessageBox::warning(this,"Theme image","Could not copy the dark image into project assets.");return;}}r->setData(AltAssetRole,target);
  }
  void rebuildTilePalette(QGraphicsRectItem *r){
    if(!tilePalette||!r||r->data(TypeRole).toString()!="Tile Map")return;QSignalBlocker guard(tilePaletteCellSize);int cell=qMax(4,r->data(CellSizeRole).toInt());tilePaletteCellSize->setValue(cell);tilePalette->clear();QPixmap source(r->data(AssetRole).toString());if(source.isNull())return;int columns=source.width()/cell,rows=source.height()/cell;for(int y=0;y<rows;y++)for(int x=0;x<columns;x++){int index=y*columns+x;QPixmap tile=source.copy(x*cell,y*cell,cell,cell);auto *item=new QListWidgetItem(QIcon(tile),QString::number(index));item->setData(Qt::UserRole,index);tilePalette->addItem(item);}int selected=r->data(TileIndexRole).toInt();if(selected>=0&&selected<tilePalette->count())tilePalette->setCurrentRow(selected);
  }
  void inspector() {
    clearForm();
    auto s = view->world.selectedItems();
    if (s.isEmpty()) {
      auto *l = new QLabel("Select an entity.\nDouble-click an object to add "
                           "it.\nDashed rectangle is the game camera.");
      l->setWordWrap(1);
      form->addRow(l);
      return;
    }
    auto *r = qgraphicsitem_cast<QGraphicsRectItem *>(s.first());
    if (!r)
      return;
    if(r->data(TypeRole).toString()=="Tile Map")rebuildTilePalette(r);
    auto *n = new QLineEdit(r->data(NameRole).toString());
    auto *x = new QDoubleSpinBox, *y = new QDoubleSpinBox;
    for (auto *z : {x, y}) {
      z->setRange(-5000, 5000);
      z->setDecimals(1);
    }
    x->setValue(r->x());
    y->setValue(r->y());
    auto *behavior = new QComboBox;
    behavior->addItems({"None", "Player Controller", "Patrol", "Rotator",
                        "Floating", "Pulse Shader", "Particle Emitter", "Bounce", "Orbit", "Blink", "Pulse Scale"});
    behavior->setCurrentText(r->data(BehaviorRole).toString());
    auto *textValue=new QLineEdit(r->data(TextRole).toString());
    auto *objectScene=new QComboBox;objectScene->addItems(sceneNames);objectScene->setCurrentText(r->data(SceneRole).toString());
    auto *targetScene=new QComboBox;targetScene->addItems(sceneNames);targetScene->setCurrentText(r->data(TargetSceneRole).toString());
    auto *shape=new QComboBox;shape->addItems({"Circle","Square","Rounded Square","Ellipse","Hexagon"});shape->setCurrentText(r->data(ShapeRole).toString());
    auto *layoutDirection=new QComboBox;layoutDirection->addItems({"Vertical","Horizontal"});layoutDirection->setCurrentText(r->data(ShapeRole).toString());
    auto *cameraAspect=new QComboBox;cameraAspect->addItems({"Follow Game","16:9","9:16","Custom"});cameraAspect->setCurrentText(r->data(ShapeRole).toString());
    auto *themeTarget=new QListWidget;themeTarget->setMaximumHeight(145);QStringList selectedTargets=r->data(TargetObjectRole).toString().split(',',Qt::SkipEmptyParts);for(auto*e:view->entities())if(e!=r&&e->data(SceneRole)==r->data(SceneRole)){auto*item=new QListWidgetItem(e->data(NameRole).toString(),themeTarget);item->setFlags(item->flags()|Qt::ItemIsUserCheckable);item->setCheckState(selectedTargets.contains(e->data(NameRole).toString())?Qt::Checked:Qt::Unchecked);}
    auto *zLayer=new QSpinBox;zLayer->setRange(-1000,1000);zLayer->setValue(r->data(ZRole).toInt());
    auto *rotation=new QDoubleSpinBox;rotation->setRange(-36000,36000);rotation->setSuffix("°");rotation->setValue(r->data(RotationRole).toDouble());
    auto *scaleValue=new QDoubleSpinBox;scaleValue->setRange(.01,100);scaleValue->setDecimals(2);scaleValue->setValue(r->data(ScaleRole).toDouble());
    auto *opacity=new QSlider(Qt::Horizontal);opacity->setRange(0,100);opacity->setValue(qRound(r->data(OpacityRole).toDouble()*100));
    auto *visibleObject=new QCheckBox("Visible in game");visibleObject->setChecked(r->data(ObjectVisibleRole).toBool());
    auto *layerButtons=new QWidget;auto *layerLayout=new QGridLayout(layerButtons);layerLayout->setContentsMargins(0,0,0,0);auto *front=new QPushButton("To Front"),*forward=new QPushButton("Forward"),*backward=new QPushButton("Backward"),*back=new QPushButton("To Back");layerLayout->addWidget(front,0,0);layerLayout->addWidget(forward,0,1);layerLayout->addWidget(backward,1,0);layerLayout->addWidget(back,1,1);
    auto *alignWidget=new QWidget;auto *alignLayout=new QGridLayout(alignWidget);alignLayout->setContentsMargins(0,0,0,0);auto *alignCenter=new QPushButton("Center"),*alignLeft=new QPushButton("Left"),*alignRight=new QPushButton("Right"),*alignTop=new QPushButton("Top"),*alignBottom=new QPushButton("Bottom");alignLayout->addWidget(alignCenter,0,0);alignLayout->addWidget(alignLeft,0,1);alignLayout->addWidget(alignRight,0,2);alignLayout->addWidget(alignTop,1,0);alignLayout->addWidget(alignBottom,1,1);
    QString assetPrompt=r->data(TypeRole)=="Video Player"?"Choose video file":(r->data(TypeRole)=="Audio Source"?"Choose audio file":"Choose image / GIF");auto *asset = new QPushButton(r->data(AssetRole).toString().isEmpty() ? assetPrompt : QFileInfo(r->data(AssetRole).toString()).fileName());
    auto *altAsset=new QPushButton(r->data(AltAssetRole).toString().isEmpty()?"Choose dark image":QFileInfo(r->data(AltAssetRole).toString()).fileName());
    auto *fit = new QComboBox;
    fit->addItems({"Fit", "Fill", "Crop", "Center"});
    fit->setCurrentText(r->data(FitRole).toString());
    auto *width = new QDoubleSpinBox, *height = new QDoubleSpinBox;
    for (auto *size : {width, height}) { size->setRange(1, 4096); size->setSuffix(" px"); }
    width->setValue(r->data(WidthRole).toDouble());
    height->setValue(r->data(HeightRole).toDouble());
    auto *font = new QFontComboBox;
    font->setCurrentFont(QFont(r->data(FontRole).toString()));
    auto spin=[&](int value,int low,int high){auto*s=new QSpinBox;s->setRange(low,high);s->setValue(value);return s;};
    auto *cellSize=spin(r->data(CellSizeRole).toInt(),4,512);
    auto *columns=spin(r->data(ColumnsRole).toInt(),1,1000);
    auto *rows=spin(r->data(RowsRole).toInt(),1,1000);
    auto *gridVisible=new QCheckBox("Render grid lines"); gridVisible->setChecked(r->data(GridVisibleRole).toBool());
    auto *gravity=new QDoubleSpinBox; gravity->setRange(-5000,5000);gravity->setValue(r->data(GravityRole).toDouble());gravity->setSuffix(" px/s²");
    auto *lockRotation=new QCheckBox("Keep character upright");lockRotation->setChecked(r->data(LockRotationRole).toBool());
    auto *cameraFollow=new QComboBox;cameraFollow->addItem("None");for(auto*e:view->entities())if(e->data(TypeRole)!="Camera")cameraFollow->addItem(e->data(NameRole).toString());cameraFollow->setCurrentText(r->data(CameraFollowRole).toString());
    auto *adaptive=new QCheckBox("Adapt to visible game window + culling");adaptive->setChecked(r->data(CameraAdaptiveRole).toBool());adaptive->setToolTip("Scales the 800 × 450 camera area to the current game window and skips objects outside the visible area.");
    auto *margin=spin(r->data(CullMarginRole).toInt(),0,2048);margin->setSuffix(" px");
    auto *up=new QKeySequenceEdit(QKeySequence(r->data(MoveUpRole).toString()));
    auto *down=new QKeySequenceEdit(QKeySequence(r->data(MoveDownRole).toString()));
    auto *left=new QKeySequenceEdit(QKeySequence(r->data(MoveLeftRole).toString()));
    auto *right=new QKeySequenceEdit(QKeySequence(r->data(MoveRightRole).toString()));
    auto *hitbox=new QCheckBox("Enable collisions");hitbox->setChecked(r->data(HitboxRole).toBool());
    auto *showHitbox=new QCheckBox("Show hitbox in game");showHitbox->setChecked(r->data(ShowHitboxRole).toBool());
    auto *blur=new QSlider(Qt::Horizontal);blur->setRange(0,100);blur->setValue(qRound(r->data(BlurRole).toDouble()));blur->setToolTip("0 = sharp solid block, 100 = very soft color glow");
    auto *cornerRadius=spin(qRound(r->data(CornerRadiusRole).toDouble()),0,512);cornerRadius->setSuffix(" px");
    auto *uiValue=spin(r->data(UiValueRole).toInt(),0,100);uiValue->setSuffix(" %");
    auto *physicsEnabled=new QCheckBox("Enable dynamic physics");physicsEnabled->setChecked(r->data(PhysicsRole).toBool());
    auto *effectMode=new QComboBox;effectMode->addItems({"Normal","Blur","Glass","Blur + Glass"});effectMode->setCurrentText(r->data(GlassRole).toBool()?(r->data(BlurRole).toDouble()>0?"Blur + Glass":"Glass"):(r->data(BlurRole).toDouble()>0?"Blur":"Normal"));
    auto *textOpacity=new QCheckBox("Text follows button opacity");textOpacity->setChecked(r->data(TextOpacityRole).toBool());
    auto *paintMode=new QCheckBox("Paint cells with mouse");paintMode->setChecked(r->data(CellPaintModeRole).toBool());auto *paintColor=new QPushButton("Choose cell paint color"),*clearCells=new QPushButton("Clear painted cells");
    auto *color = new QPushButton("Choose color");
    auto *fillColor=new QPushButton("Choose fill color"),*strokeColor=new QPushButton("Choose outline color");
    auto *pressEnabled=new QCheckBox("Enable press highlight");pressEnabled->setChecked(r->data(PressEffectRole).toBool());auto *pressColor=new QPushButton("Choose pressed color");
    auto *autoPlay=new QCheckBox("Play automatically");autoPlay->setChecked(r->data(AutoPlayRole).toBool());auto *mediaLoop=new QCheckBox("Loop playback");mediaLoop->setChecked(r->data(LoopRole).toBool());auto *volume=spin(r->data(VolumeRole).toInt(),0,100);volume->setSuffix(" %");
    auto *saveKey=new QLineEdit(r->data(SaveKeyRole).toString());auto *saveFields=new QLineEdit(r->data(SaveFieldsRole).toString());saveFields->setPlaceholderText("Player.x,Score.text,Coins.uiValue");auto *autoLoad=new QCheckBox("Load automatically on start");autoLoad->setChecked(r->data(AutoLoadRole).toBool());
    auto *vibrationDuration=spin(r->data(VibrationDurationRole).toInt(),1,5000);vibrationDuration->setSuffix(" ms");auto *vibrationOnStart=new QCheckBox("Vibrate on scene start");vibrationOnStart->setChecked(r->data(VibrationOnStartRole).toBool());
    auto *radioGroup=new QLineEdit(r->data(RadioGroupRole).toString());
    auto *del = new QPushButton("Delete object");
    form->addRow("Object", n);
    form->addRow("Type", new QLabel(r->data(TypeRole).toString()));
    form->addRow("X", x);
    form->addRow("Y", y);
    form->addRow("Attached script", behavior);
    form->addRow("Object scene",objectScene);
    form->addRow("Z Layer",zLayer);form->addRow("Layer order",layerButtons);form->addRow("Align to camera",alignWidget);form->addRow("Rotation",rotation);form->addRow("Scale",scaleValue);form->addRow("Opacity",opacity);form->addRow(visibleObject);
    if(r->data(TypeRole)=="Text"||r->data(TypeRole)=="Button"||r->data(TypeRole)=="Button Container"||r->data(TypeRole)=="UI Container"||r->data(TypeRole)=="Dropdown"||r->data(TypeRole)=="Check Box"||r->data(TypeRole)=="Radio Button"||r->data(TypeRole)=="Scroll View")form->addRow(r->data(TypeRole)=="Dropdown"?"Options (comma separated)":(r->data(TypeRole).toString().contains("Container")?"Buttons (comma separated)":(r->data(TypeRole)=="Scroll View"?"Items (comma separated)":"Displayed text")),textValue);
    if(r->data(TypeRole)=="Button"||r->data(TypeRole)=="Scene Portal")form->addRow("On click → scene",targetScene);
    form->addRow(r->data(TypeRole)=="Video Player"?"Video file":(r->data(TypeRole)=="Audio Source"?"Audio file":(r->data(TypeRole)=="Theme Switch"?"Light image":"Image")), asset);
    if(r->data(TypeRole)=="Theme Switch"){form->addRow("Theme targets",themeTarget);form->addRow("Dark image",altAsset);form->addRow("Light color",color);form->addRow("Dark color",fillColor);}
    form->addRow("Image mode", fit);
    form->addRow("Width", width);
    form->addRow("Height", height);
    form->addRow(hitbox);form->addRow(showHitbox);
    if(r->data(TypeRole)!="Camera"&&r->data(TypeRole)!="Audio Source"&&r->data(TypeRole)!="Video Player"){form->addRow("Visual effect",effectMode);form->addRow("Blur strength",blur);form->addRow("Corner radius",cornerRadius);}
    if(r->data(TypeRole)=="Button")form->addRow(textOpacity);
    if(r->data(TypeRole)=="Button"||r->data(TypeRole)=="Dropdown"||r->data(TypeRole)=="Toggle"||r->data(TypeRole)=="Check Box"||r->data(TypeRole)=="Slider"||r->data(TypeRole)=="Theme Switch"){form->addRow(pressEnabled);form->addRow("Pressed highlight",pressColor);}
    if(r->data(TypeRole)=="Physics Block"||r->data(TypeRole)=="Character"){form->addRow(physicsEnabled);form->addRow("Physics gravity",gravity);}
    if (r->data(TypeRole) == "Text" || r->data(TypeRole) == "Button") form->addRow("Custom font", font);
    if (r->data(TypeRole) == "Grid Map"||r->data(TypeRole)=="Tile Map") {form->addRow("Cell size",cellSize);form->addRow("Columns",columns);form->addRow("Rows",rows);form->addRow(gridVisible);form->addRow(paintMode);form->addRow(paintColor);form->addRow(clearCells);form->addRow("Map fill",fillColor);form->addRow("Grid/outline",strokeColor);}
    if (r->data(TypeRole) == "ASFO Territory") {form->addRow("Territory shape",shape);form->addRow("Territory fill",fillColor);form->addRow("Territory outline",strokeColor);}
    if(r->data(TypeRole)=="Button Container"||r->data(TypeRole)=="UI Container")form->addRow("Button layout",layoutDirection);
    if(r->data(TypeRole)=="Toggle"||r->data(TypeRole)=="Check Box"||r->data(TypeRole)=="Radio Button"){auto *togglePreview=new QCheckBox("Initial state is ON");togglePreview->setChecked(r->data(UiStateRole).toBool());form->addRow(togglePreview);connect(togglePreview,&QCheckBox::toggled,this,[this,r](bool on){r->setData(UiStateRole,on);view->viewport()->update();});}
    if(r->data(TypeRole)=="Radio Button")form->addRow("Radio group",radioGroup);
    if(r->data(TypeRole)=="Video Player"||r->data(TypeRole)=="Audio Source"){form->addRow(autoPlay);form->addRow(mediaLoop);form->addRow("Volume",volume);}
    if(r->data(TypeRole)=="Save Data"){form->addRow("Save slot",saveKey);form->addRow("Remember properties",saveFields);form->addRow(autoLoad);}
    if(r->data(TypeRole)=="Vibration"){form->addRow("Duration",vibrationDuration);form->addRow(vibrationOnStart);}
    if(r->data(TypeRole)=="Slider"||r->data(TypeRole)=="Progress Bar"||r->data(TypeRole)=="Scroll View")form->addRow(r->data(TypeRole)=="Scroll View"?"Scroll position":"Value",uiValue);
    if (r->data(TypeRole) == "Character") {form->addRow(lockRotation);form->addRow("Move Up",up);form->addRow("Move Down",down);form->addRow("Move Left",left);form->addRow("Move Right",right);}
    if (r->data(TypeRole) == "Camera") {form->addRow("Camera aspect",cameraAspect);form->addRow("Follow object",cameraFollow);form->addRow(adaptive);form->addRow("Cull margin",margin);}
    if(r->data(TypeRole)!="Theme Switch")form->addRow("Material", color);
    form->addRow(del);
    connect(n, &QLineEdit::textChanged, this, [this, r](QString z) {
      r->setData(NameRole, z);
      for (auto *c : r->childItems())
        if (auto *t = qgraphicsitem_cast<QGraphicsSimpleTextItem *>(c)) {
          t->setText(z);
          t->setPos(-t->boundingRect().width() / 2, 38);
        }
      hierarchy();
    });
    connect(x, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            [r](double z) { r->setX(z); });
    connect(y, qOverload<double>(&QDoubleSpinBox::valueChanged), this,
            [r](double z) { r->setY(z); });
    connect(behavior, &QComboBox::currentTextChanged, this,
            [this, r](QString z) {
              r->setData(BehaviorRole, z);
              r->setData(ScriptRole, z);
              hierarchy();
            });
    connect(textValue,&QLineEdit::textChanged,this,[r](const QString&v){r->setData(TextRole,v);});
    connect(objectScene,&QComboBox::currentTextChanged,this,[this,r](const QString&v){r->setData(SceneRole,v);r->setVisible(v==currentScene&&r->data(ObjectVisibleRole).toBool());hierarchy();});
    connect(targetScene,&QComboBox::currentTextChanged,this,[r](const QString&v){r->setData(TargetSceneRole,v);});
    connect(shape,&QComboBox::currentTextChanged,this,[this,r](const QString&v){r->setData(ShapeRole,v);view->viewport()->update();});
    connect(layoutDirection,&QComboBox::currentTextChanged,this,[this,r](const QString&v){r->setData(ShapeRole,v);view->viewport()->update();});
    auto setLayer=[this,r,zLayer](int z){z=qBound(-1000,z,1000);r->setData(ZRole,z);r->setZValue(z);zLayer->setValue(z);view->viewport()->update();};
    connect(zLayer,qOverload<int>(&QSpinBox::valueChanged),this,[setLayer](int z){setLayer(z);});connect(front,&QPushButton::clicked,this,[setLayer]{setLayer(1000);});connect(back,&QPushButton::clicked,this,[setLayer]{setLayer(-1000);});connect(forward,&QPushButton::clicked,this,[r,setLayer]{setLayer(r->data(ZRole).toInt()+1);});connect(backward,&QPushButton::clicked,this,[r,setLayer]{setLayer(r->data(ZRole).toInt()-1);});
    connect(alignCenter,&QPushButton::clicked,this,[r]{r->setPos(0,0);});connect(alignLeft,&QPushButton::clicked,this,[r]{r->setX(-400+r->data(WidthRole).toDouble()/2);});connect(alignRight,&QPushButton::clicked,this,[r]{r->setX(400-r->data(WidthRole).toDouble()/2);});connect(alignTop,&QPushButton::clicked,this,[r]{r->setY(-225+r->data(HeightRole).toDouble()/2);});connect(alignBottom,&QPushButton::clicked,this,[r]{r->setY(225-r->data(HeightRole).toDouble()/2);});
    connect(rotation,qOverload<double>(&QDoubleSpinBox::valueChanged),this,[r](double v){r->setData(RotationRole,v);r->setRotation(v);});connect(scaleValue,qOverload<double>(&QDoubleSpinBox::valueChanged),this,[r](double v){r->setData(ScaleRole,v);r->setScale(v);});connect(opacity,&QSlider::valueChanged,this,[r](int v){r->setData(OpacityRole,v/100.0);r->setOpacity(v/100.0);});connect(visibleObject,&QCheckBox::toggled,this,[this,r](bool v){r->setData(ObjectVisibleRole,v);r->setVisible(v&&r->data(SceneRole).toString()==currentScene);});
    connect(asset, &QPushButton::clicked, this, [this, r, asset] {chooseImage(r);if(!r->data(AssetRole).toString().isEmpty())asset->setText(QFileInfo(r->data(AssetRole).toString()).fileName());});
    connect(altAsset,&QPushButton::clicked,this,[this,r,altAsset]{chooseAlternateImage(r);if(!r->data(AltAssetRole).toString().isEmpty())altAsset->setText(QFileInfo(r->data(AltAssetRole).toString()).fileName());});
    connect(themeTarget,&QListWidget::itemChanged,this,[r,themeTarget](QListWidgetItem*){QStringList names;for(int i=0;i<themeTarget->count();++i)if(themeTarget->item(i)->checkState()==Qt::Checked)names<<themeTarget->item(i)->text();r->setData(TargetObjectRole,names.join(','));});
    connect(cameraAspect,&QComboBox::currentTextChanged,this,[this,r,width,height](const QString&mode){r->setData(ShapeRole,mode);QSignalBlocker bw(width),bh(height);if(mode=="Follow Game"){r->setData(WidthRole,view->portraitMode?450.0:800.0);r->setData(HeightRole,view->portraitMode?800.0:450.0);}else if(mode=="16:9"){r->setData(WidthRole,800.0);r->setData(HeightRole,450.0);}else if(mode=="9:16"){r->setData(WidthRole,450.0);r->setData(HeightRole,800.0);}width->setValue(r->data(WidthRole).toDouble());height->setValue(r->data(HeightRole).toDouble());view->viewport()->update();});
    connect(fit, &QComboBox::currentTextChanged, this, [r](const QString &v){ r->setData(FitRole,v); });
    connect(width, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [r](double v){ r->setData(WidthRole,v);if(r->data(TypeRole)=="Grid Map"||r->data(TypeRole)=="Tile Map"||r->data(TypeRole)=="Camera")return;r->setRect(-v/2,r->rect().y(),v,r->rect().height()); });
    connect(height, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [r](double v){ r->setData(HeightRole,v);if(r->data(TypeRole)=="Grid Map"||r->data(TypeRole)=="Tile Map"||r->data(TypeRole)=="Camera")return;r->setRect(r->rect().x(),-v/2,r->rect().width(),v); });
    connect(font, &QFontComboBox::currentFontChanged, this, [r](const QFont &v){ r->setData(FontRole,v.family()); });
    connect(fillColor,&QPushButton::clicked,this,[this,r]{QColor c=QColorDialog::getColor(r->data(FillColorRole).value<QColor>(),this,"Map fill");if(c.isValid()){r->setData(FillColorRole,c);view->viewport()->update();}});
    connect(strokeColor,&QPushButton::clicked,this,[this,r]{QColor c=QColorDialog::getColor(r->data(StrokeColorRole).value<QColor>(),this,"Map outline");if(c.isValid()){r->setData(StrokeColorRole,c);view->viewport()->update();}});
    connect(pressEnabled,&QCheckBox::toggled,this,[r](bool enabled){r->setData(PressEffectRole,enabled);});connect(pressColor,&QPushButton::clicked,this,[this,r]{QColor c=QColorDialog::getColor(r->data(PressColorRole).value<QColor>(),this,"Button pressed highlight");if(c.isValid())r->setData(PressColorRole,c);});
    connect(autoPlay,&QCheckBox::toggled,this,[r](bool v){r->setData(AutoPlayRole,v);});connect(mediaLoop,&QCheckBox::toggled,this,[r](bool v){r->setData(LoopRole,v);});connect(volume,qOverload<int>(&QSpinBox::valueChanged),this,[r](int v){r->setData(VolumeRole,v);});connect(saveKey,&QLineEdit::textChanged,this,[r](const QString&v){r->setData(SaveKeyRole,v);});connect(saveFields,&QLineEdit::textChanged,this,[r](const QString&v){r->setData(SaveFieldsRole,v);});connect(autoLoad,&QCheckBox::toggled,this,[r](bool v){r->setData(AutoLoadRole,v);});connect(vibrationDuration,qOverload<int>(&QSpinBox::valueChanged),this,[r](int v){r->setData(VibrationDurationRole,v);});connect(vibrationOnStart,&QCheckBox::toggled,this,[r](bool v){r->setData(VibrationOnStartRole,v);});connect(radioGroup,&QLineEdit::textChanged,this,[r](const QString&v){r->setData(RadioGroupRole,v);});
    auto sync=[this,r]{syncCodeFromInspector(r);view->viewport()->update();};
    connect(cellSize,qOverload<int>(&QSpinBox::valueChanged),this,[r,sync,columns,rows](int v){r->setData(CellSizeRole,v);if(r->data(TypeRole)=="Grid Map"||r->data(TypeRole)=="Tile Map"){int w=columns->value()*v,h=rows->value()*v;r->setData(WidthRole,w);r->setData(HeightRole,h);r->setRect(0,0,w,h);}sync();});
    connect(columns,qOverload<int>(&QSpinBox::valueChanged),this,[r,sync,cellSize,rows](int v){r->setData(ColumnsRole,v);if(r->data(TypeRole)=="Grid Map"||r->data(TypeRole)=="Tile Map"){int w=v*cellSize->value(),h=rows->value()*cellSize->value();r->setData(WidthRole,w);r->setData(HeightRole,h);r->setRect(0,0,w,h);}sync();});
    connect(rows,qOverload<int>(&QSpinBox::valueChanged),this,[r,sync,cellSize,columns](int v){r->setData(RowsRole,v);if(r->data(TypeRole)=="Grid Map"||r->data(TypeRole)=="Tile Map"){int w=columns->value()*cellSize->value(),h=v*cellSize->value();r->setData(WidthRole,w);r->setData(HeightRole,h);r->setRect(0,0,w,h);}sync();});
    connect(gridVisible,&QCheckBox::toggled,this,[r,sync](bool v){r->setData(GridVisibleRole,v);sync();});
    connect(gravity,qOverload<double>(&QDoubleSpinBox::valueChanged),this,[r,sync](double v){r->setData(GravityRole,v);sync();});
    connect(lockRotation,&QCheckBox::toggled,this,[r,sync](bool v){r->setData(LockRotationRole,v);sync();});
    connect(cameraFollow,&QComboBox::currentTextChanged,this,[r,sync](const QString&v){r->setData(CameraFollowRole,v);sync();});
    connect(adaptive,&QCheckBox::toggled,this,[r,sync](bool v){r->setData(CameraAdaptiveRole,v);sync();});
    connect(margin,qOverload<int>(&QSpinBox::valueChanged),this,[r,sync](int v){r->setData(CullMarginRole,v);sync();});
    connect(up,&QKeySequenceEdit::keySequenceChanged,this,[r](const QKeySequence&v){r->setData(MoveUpRole,v.toString());});
    connect(down,&QKeySequenceEdit::keySequenceChanged,this,[r](const QKeySequence&v){r->setData(MoveDownRole,v.toString());});
    connect(left,&QKeySequenceEdit::keySequenceChanged,this,[r](const QKeySequence&v){r->setData(MoveLeftRole,v.toString());});
    connect(right,&QKeySequenceEdit::keySequenceChanged,this,[r](const QKeySequence&v){r->setData(MoveRightRole,v.toString());});
    connect(hitbox,&QCheckBox::toggled,this,[r](bool v){r->setData(HitboxRole,v);});
    connect(showHitbox,&QCheckBox::toggled,this,[r](bool v){r->setData(ShowHitboxRole,v);});
    connect(blur,&QSlider::valueChanged,this,[this,r](int v){r->setData(BlurRole,(double)v);view->viewport()->update();});
    connect(cornerRadius,qOverload<int>(&QSpinBox::valueChanged),this,[this,r](int v){r->setData(CornerRadiusRole,(double)v);view->viewport()->update();});
    connect(uiValue,qOverload<int>(&QSpinBox::valueChanged),this,[this,r](int v){r->setData(UiValueRole,v);view->viewport()->update();});
    connect(physicsEnabled,&QCheckBox::toggled,this,[r](bool v){r->setData(PhysicsRole,v);});
    connect(effectMode,&QComboBox::currentTextChanged,this,[this,r,blur](const QString&v){bool glass=v.contains("Glass"),soft=v.contains("Blur");r->setData(GlassRole,glass);if(soft&&r->data(BlurRole).toDouble()<=0){r->setData(BlurRole,45.0);QSignalBlocker guard(blur);blur->setValue(45);}if(!soft){r->setData(BlurRole,0.0);QSignalBlocker guard(blur);blur->setValue(0);}view->viewport()->update();});connect(textOpacity,&QCheckBox::toggled,this,[r](bool v){r->setData(TextOpacityRole,v);});connect(paintMode,&QCheckBox::toggled,this,[r](bool v){r->setData(CellPaintModeRole,v);});connect(paintColor,&QPushButton::clicked,this,[this,r]{QColor c=QColorDialog::getColor(r->data(CellPaintColorRole).value<QColor>(),this,"Cell paint color");if(c.isValid())r->setData(CellPaintColorRole,c);});connect(clearCells,&QPushButton::clicked,this,[this,r]{r->setData(CellColorsRole,QVariantMap());r->setData(TileCellsRole,QVariantMap());view->viewport()->update();});
    connect(color, &QPushButton::clicked, this, [this, r] {
      QColor c = QColorDialog::getColor(r->data(ColorRole).value<QColor>(),
                                        this, "Entity material");
      if (c.isValid()) {
        r->setData(ColorRole, c);
        r->setBrush(c.darker(170));
        r->setPen(QPen(c.lighter(150), 2));
      }
    });
    connect(del, &QPushButton::clicked, this, [this, r] {
      delete r;
      hierarchy();
      inspector();
    });
  }
  QString dir() const { return QCoreApplication::applicationDirPath(); }
  void createProject(){
    QString parent=QFileDialog::getExistingDirectory(this,"Choose folder for projects",QDir::homePath());if(parent.isEmpty())return;
    bool ok=false;QString name=QInputDialog::getText(this,"Create project","Project folder name:",QLineEdit::Normal,"MyGame",&ok).trimmed();if(!ok||name.isEmpty())return;
    name.replace(QRegularExpression(R"([\\/:*?"<>|])"),"_");QString target=QDir(parent).filePath(name);if(!QDir().mkpath(target)){QMessageBox::critical(this,"Create project","Cannot create project folder.");return;}
    projectDir=QDir(target).absolutePath();save();setWindowTitle("Engine++ Studio — "+name);statusBar()->showMessage("Project folder created: "+projectDir,6000);
  }
  void chooseLoadProject(){
    QString folder=QFileDialog::getExistingDirectory(this,"Open Engine++ project folder",projectDir);if(folder.isEmpty())return;
    if(!QFileInfo::exists(QDir(folder).filePath("project.enginepp.json"))){QMessageBox::information(this,"Open project","This folder has no project.enginepp.json file.");return;}
    projectDir=QDir(folder).absolutePath();loadProject();setWindowTitle("Engine++ Studio — "+QFileInfo(projectDir).fileName());
  }
  void save() {
    scripts[currentScript]=code->toPlainText();QDir().mkpath(projectDir+"/scripts");QDir().mkpath(projectDir+"/assets");
    for(auto it=scripts.begin();it!=scripts.end();++it){QFile f(projectDir+"/scripts/"+it.key());if(f.open(QIODevice::WriteOnly))f.write(it.value().toUtf8());}
    QJsonObject project;project["version"]=4;project["currentScene"]=currentScene;project["androidFullscreen"]=androidFullscreen;project["gameOrientation"]=gameOrientation;project["gamePixelWidth"]=gamePixelWidth;project["gamePixelHeight"]=gamePixelHeight;project["androidSystemBarColor"]=androidSystemBarColor.name(QColor::HexArgb);project["gameName"]=gameName;project["gameIconPath"]=gameIconPath;QJsonArray sceneArray;for(const auto&s:sceneNames)sceneArray.append(s);project["scenes"]=sceneArray;
    QJsonObject scriptObject;for(auto it=scripts.begin();it!=scripts.end();++it)scriptObject[it.key()]=it.value();project["scripts"]=scriptObject;QJsonObject targetObject;for(auto it=scriptTargets.begin();it!=scriptTargets.end();++it)targetObject[it.key()]=it.value();project["scriptTargets"]=targetObject;
    QJsonArray objects;for(auto*r:view->entities()){QJsonObject o;o["name"]=r->data(NameRole).toString();o["type"]=r->data(TypeRole).toString();o["x"]=r->x();o["y"]=r->y();o["width"]=r->data(WidthRole).toDouble();o["height"]=r->data(HeightRole).toDouble();o["scene"]=r->data(SceneRole).toString();o["targetScene"]=r->data(TargetSceneRole).toString();o["text"]=r->data(TextRole).toString();o["shape"]=r->data(ShapeRole).toString();o["script"]=r->data(ScriptRole).toString();o["behavior"]=r->data(BehaviorRole).toString();o["asset"]=r->data(AssetRole).toString();o["fit"]=r->data(FitRole).toString();o["color"]=r->data(ColorRole).value<QColor>().name(QColor::HexArgb);o["fill"]=r->data(FillColorRole).value<QColor>().name(QColor::HexArgb);o["stroke"]=r->data(StrokeColorRole).value<QColor>().name(QColor::HexArgb);o["cellSize"]=r->data(CellSizeRole).toInt();o["columns"]=r->data(ColumnsRole).toInt();o["rows"]=r->data(RowsRole).toInt();o["gridVisible"]=r->data(GridVisibleRole).toBool();o["gravity"]=r->data(GravityRole).toDouble();o["lockRotation"]=r->data(LockRotationRole).toBool();o["hitbox"]=r->data(HitboxRole).toBool();o["showHitbox"]=r->data(ShowHitboxRole).toBool();o["cameraFollow"]=r->data(CameraFollowRole).toString();o["cameraAdaptive"]=r->data(CameraAdaptiveRole).toBool();o["cullMargin"]=r->data(CullMarginRole).toInt();o["up"]=r->data(MoveUpRole).toString();o["down"]=r->data(MoveDownRole).toString();o["left"]=r->data(MoveLeftRole).toString();o["right"]=r->data(MoveRightRole).toString();o["z"]=r->data(ZRole).toInt();o["rotation"]=r->data(RotationRole).toDouble();o["scale"]=r->data(ScaleRole).toDouble();o["opacity"]=r->data(OpacityRole).toDouble();o["objectVisible"]=r->data(ObjectVisibleRole).toBool();objects.append(o);}project["objects"]=objects;
    {auto es=view->entities();for(int i=0;i<objects.size()&&i<es.size();++i){QJsonObject o=objects[i].toObject();o["glass"]=es[i]->data(GlassRole).toBool();o["blur"]=es[i]->data(BlurRole).toDouble();o["physics"]=es[i]->data(PhysicsRole).toBool();o["textOpacity"]=es[i]->data(TextOpacityRole).toBool();o["uiState"]=es[i]->data(UiStateRole).toBool();o["tileIndex"]=es[i]->data(TileIndexRole).toInt();o["targetObject"]=es[i]->data(TargetObjectRole).toString();o["altAsset"]=es[i]->data(AltAssetRole).toString();o["cornerRadius"]=es[i]->data(CornerRadiusRole).toDouble();o["uiValue"]=es[i]->data(UiValueRole).toInt();o["pressColor"]=es[i]->data(PressColorRole).value<QColor>().name(QColor::HexArgb);o["pressEffect"]=es[i]->data(PressEffectRole).toBool();o["autoPlay"]=es[i]->data(AutoPlayRole).toBool();o["loop"]=es[i]->data(LoopRole).toBool();o["volume"]=es[i]->data(VolumeRole).toInt();o["saveKey"]=es[i]->data(SaveKeyRole).toString();o["saveFields"]=es[i]->data(SaveFieldsRole).toString();o["autoLoad"]=es[i]->data(AutoLoadRole).toBool();o["vibrationDuration"]=es[i]->data(VibrationDurationRole).toInt();o["vibrationOnStart"]=es[i]->data(VibrationOnStartRole).toBool();o["radioGroup"]=es[i]->data(RadioGroupRole).toString();QJsonObject cells;QVariantMap map=es[i]->data(CellColorsRole).toMap();for(auto it=map.begin();it!=map.end();++it)cells[it.key()]=it.value().value<QColor>().name(QColor::HexArgb);o["cellColors"]=cells;QJsonObject tiles;QVariantMap tileMap=es[i]->data(TileCellsRole).toMap();for(auto it=tileMap.begin();it!=tileMap.end();++it)tiles[it.key()]=it.value().toInt();o["tileCells"]=tiles;objects[i]=o;}project["objects"]=objects;}
    QFile projectFile(projectDir+"/project.enginepp.json");if(projectFile.open(QIODevice::WriteOnly))projectFile.write(QJsonDocument(project).toJson(QJsonDocument::Indented));
    statusBar()->showMessage(QString("Project saved • %1 objects • %2 scripts • %3").arg(objects.size()).arg(scripts.size()).arg(projectDir), 5000);
  }
  void loadProject(){QFile f(projectDir+"/project.enginepp.json");if(!f.open(QIODevice::ReadOnly)){QMessageBox::information(this,"Load project","project.enginepp.json was not found in the selected project folder.");return;}QJsonDocument doc=QJsonDocument::fromJson(f.readAll());if(!doc.isObject()){QMessageBox::critical(this,"Load project","Project file is invalid.");return;}QJsonObject p=doc.object();view->world.clear();scripts.clear();scriptTargets.clear();scriptList->clear();sceneNames.clear();for(auto v:p["scenes"].toArray())sceneNames<<v.toString();if(sceneNames.isEmpty())sceneNames<<"Scene 1";QSignalBlocker comboGuard(sceneCombo);sceneCombo->clear();sceneCombo->addItems(sceneNames);sceneCombo->addItem(newSceneEntry());QJsonObject loadedScripts=p["scripts"].toObject(),loadedTargets=p["scriptTargets"].toObject();for(auto it=loadedScripts.begin();it!=loadedScripts.end();++it){scripts[it.key()]=it.value().toString();scriptList->addItem(it.key());}for(auto it=loadedTargets.begin();it!=loadedTargets.end();++it)scriptTargets[it.key()]=it.value().toString();if(scripts.isEmpty()){scripts["Main.cpp"]=script;scriptList->addItem("Main.cpp");}
    for(auto value:p["objects"].toArray()){QJsonObject o=value.toObject();view->add(o["type"].toString());auto*r=view->entities().last();auto set=[&](int role,const char*key){r->setData(role,o[key].toVariant());};set(NameRole,"name");set(SceneRole,"scene");set(TargetSceneRole,"targetScene");set(TextRole,"text");set(ShapeRole,"shape");set(ScriptRole,"script");set(BehaviorRole,"behavior");set(AssetRole,"asset");set(FitRole,"fit");set(WidthRole,"width");set(HeightRole,"height");set(CellSizeRole,"cellSize");set(ColumnsRole,"columns");set(RowsRole,"rows");set(GridVisibleRole,"gridVisible");set(GravityRole,"gravity");set(LockRotationRole,"lockRotation");set(HitboxRole,"hitbox");set(ShowHitboxRole,"showHitbox");set(CameraFollowRole,"cameraFollow");set(CameraAdaptiveRole,"cameraAdaptive");set(CullMarginRole,"cullMargin");set(MoveUpRole,"up");set(MoveDownRole,"down");set(MoveLeftRole,"left");set(MoveRightRole,"right");r->setData(ZRole,o["z"].toInt(0));r->setData(RotationRole,o["rotation"].toDouble(0));r->setData(ScaleRole,o["scale"].toDouble(1));r->setData(OpacityRole,o["opacity"].toDouble(1));r->setData(ObjectVisibleRole,o.contains("objectVisible")?o["objectVisible"].toBool():true);r->setData(ColorRole,QColor(o["color"].toString()));r->setData(FillColorRole,QColor(o["fill"].toString()));r->setData(StrokeColorRole,QColor(o["stroke"].toString()));r->setPos(o["x"].toDouble(),o["y"].toDouble());r->setZValue(r->data(ZRole).toInt());r->setRotation(r->data(RotationRole).toDouble());r->setScale(r->data(ScaleRole).toDouble());r->setOpacity(r->data(OpacityRole).toDouble());double w=o["width"].toDouble(60),h=o["height"].toDouble(60);if(o["type"]=="Grid Map")r->setRect(0,0,o["columns"].toInt()*o["cellSize"].toInt(),o["rows"].toInt()*o["cellSize"].toInt());else r->setRect(-w/2,-h/2,w,h);for(auto*c:r->childItems())if(auto*t=qgraphicsitem_cast<QGraphicsSimpleTextItem*>(c))t->setText(o["name"].toString());}
    {auto entities=view->entities();auto saved=p["objects"].toArray();for(int i=0;i<entities.size()&&i<saved.size();++i){auto *e=entities[i];QJsonObject o=saved[i].toObject();e->setData(UiStateRole,o["uiState"].toBool(false));e->setData(GlassRole,o["glass"].toBool(false));e->setData(BlurRole,o["blur"].toDouble(0));e->setData(PhysicsRole,o["physics"].toBool(false));e->setData(TextOpacityRole,o.contains("textOpacity")?o["textOpacity"].toBool():true);e->setData(TileIndexRole,o["tileIndex"].toInt(0));e->setData(TargetObjectRole,o["targetObject"].toString());e->setData(AltAssetRole,o["altAsset"].toString());e->setData(CornerRadiusRole,o.contains("cornerRadius")?o["cornerRadius"].toDouble():10.0);e->setData(UiValueRole,o.contains("uiValue")?o["uiValue"].toInt():50);e->setData(PressColorRole,QColor(o["pressColor"].toString("#ffa997ff")));e->setData(PressEffectRole,o.contains("pressEffect")?o["pressEffect"].toBool():true);e->setData(AutoPlayRole,o.contains("autoPlay")?o["autoPlay"].toBool():true);e->setData(LoopRole,o["loop"].toBool(false));e->setData(VolumeRole,o.contains("volume")?o["volume"].toInt():80);e->setData(SaveKeyRole,o["saveKey"].toString("MainSave"));e->setData(SaveFieldsRole,o["saveFields"].toString());e->setData(AutoLoadRole,o.contains("autoLoad")?o["autoLoad"].toBool():true);e->setData(VibrationDurationRole,o.contains("vibrationDuration")?o["vibrationDuration"].toInt():120);e->setData(VibrationOnStartRole,o["vibrationOnStart"].toBool(false));e->setData(RadioGroupRole,o["radioGroup"].toString("Default"));QVariantMap tiles;QJsonObject tileObject=o["tileCells"].toObject();for(auto it=tileObject.begin();it!=tileObject.end();++it)tiles[it.key()]=it.value().toInt();e->setData(TileCellsRole,tiles);QVariantMap colors;QJsonObject colorObject=o["cellColors"].toObject();for(auto it=colorObject.begin();it!=colorObject.end();++it)colors[it.key()]=QColor(it.value().toString());e->setData(CellColorsRole,colors);}}
    gamePixelWidth=p["gamePixelWidth"].toInt(1280);gamePixelHeight=p["gamePixelHeight"].toInt(720);gameOrientation=gamePixelHeight>gamePixelWidth?"Portrait":(gamePixelHeight==gamePixelWidth?"Square":"Landscape");androidSystemBarColor=QColor(p["androidSystemBarColor"].toString("#ff0b0f18"));gameName=p["gameName"].toString("Engine Plus Plus Game");gameIconPath=p["gameIconPath"].toString();{QSignalBlocker orientationGuard(orientationCombo);int found=-1;QString prefix=QString::number(gamePixelWidth)+" × "+QString::number(gamePixelHeight);for(int i=0;i<orientationCombo->count()-1;i++)if(orientationCombo->itemText(i).startsWith(prefix)){found=i;break;}if(found<0){found=orientationCombo->count()-1;orientationCombo->setItemText(found,"CUSTOM  "+prefix);}orientationCombo->setCurrentIndex(found);}view->portraitMode=gamePixelHeight>gamePixelWidth;
    for(auto*e:view->entities()){if(e->data(BehaviorRole).toString().isEmpty())e->setData(BehaviorRole,e->data(ScriptRole));QString imagePath=e->data(AssetRole).toString();if(!imagePath.isEmpty()){QPixmap px(imagePath);if(!px.isNull()){QPixmap preview=px.scaled(e->rect().size().toSize(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);QBrush brush(preview);brush.setTransform(QTransform::fromTranslate(e->rect().left(),e->rect().top()));e->setBrush(brush);}}}
    currentScene=p["currentScene"].toString(sceneNames.first());sceneCombo->setCurrentText(currentScene);for(auto*e:view->entities())e->setVisible(e->data(SceneRole).toString()==currentScene&&e->data(ObjectVisibleRole).toBool());currentScript=scripts.contains("Main.cpp")?"Main.cpp":scripts.firstKey();scriptList->setCurrentRow(scriptList->row(scriptList->findItems(currentScript,Qt::MatchExactly).value(0)));QSignalBlocker guard(code);code->setPlainText(scripts.value(currentScript));hierarchy();inspector();statusBar()->showMessage("Project loaded",4000);}
  QString combinedScripts(){scripts[currentScript]=code->toPlainText();QString out;for(auto it=scripts.begin();it!=scripts.end();++it)out+="\n// ===== "+it.key()+" =====\n"+it.value()+"\n";return out;}
  static QString esc(QString s) {
    return s.replace("\\", "\\\\").replace("\"", "\\\"");
  }
  QString runtime(bool android=false) {
    QString ents;
    for (auto *r : view->entities()) {
      QColor c = r->data(ColorRole).value<QColor>();
      QColor fill=r->data(FillColorRole).value<QColor>(), stroke=r->data(StrokeColorRole).value<QColor>();
      QString assetPath=r->data(AssetRole).toString(),altAssetPath=r->data(AltAssetRole).toString();if(android&&!assetPath.isEmpty())assetPath=":/assets/"+QFileInfo(assetPath).fileName();if(android&&!altAssetPath.isEmpty())altAssetPath=":/assets/"+QFileInfo(altAssetPath).fileName();
      ents += QString("scene.e.push_back({\"%1\",\"%2\",%3,%4,0,0,0,QColor(%5,%"
                      "6,%7),true,\"%8\",\"%9\",\"%10\",%11,%12,\"%13\",%14,%15,%16,%17,QColor(%18,%19,%20),QColor(%21,%22,%23),%24,%25,\"%26\",%27,%28,\"%29\",\"%30\",\"%31\",\"%32\",%33,%34,\"%35\",\"%36\",\"%37\",\"%38\"});\n")
                  .arg(esc(r->data(NameRole).toString()),
                       esc(r->data(TypeRole).toString()))
                  .arg(r->x())
                  .arg(r->y())
                  .arg(c.red())
                  .arg(c.green())
                  .arg(c.blue())
                  .arg(esc(r->data(BehaviorRole).toString()))
                  .arg(esc(assetPath))
                  .arg(esc(r->data(FitRole).toString()))
                  .arg(r->data(WidthRole).toDouble())
                  .arg(r->data(HeightRole).toDouble())
                  .arg(esc(r->data(FontRole).toString()))
                  .arg(r->data(CellSizeRole).toInt()).arg(r->data(ColumnsRole).toInt()).arg(r->data(RowsRole).toInt())
                  .arg(r->data(GridVisibleRole).toBool()?"true":"false")
                  .arg(fill.red()).arg(fill.green()).arg(fill.blue()).arg(stroke.red()).arg(stroke.green()).arg(stroke.blue())
                  .arg(r->data(GravityRole).toDouble()).arg(r->data(LockRotationRole).toBool()?"true":"false")
                  .arg(esc(r->data(CameraFollowRole).toString())).arg(r->data(CameraAdaptiveRole).toBool()?"true":"false")
                  .arg(r->data(CullMarginRole).toInt()).arg(esc(r->data(MoveUpRole).toString())).arg(esc(r->data(MoveDownRole).toString()))
                  .arg(esc(r->data(MoveLeftRole).toString())).arg(esc(r->data(MoveRightRole).toString()))
                  .arg(r->data(HitboxRole).toBool()?"true":"false").arg(r->data(ShowHitboxRole).toBool()?"true":"false")
                  .arg(esc(r->data(TextRole).toString())).arg(esc(r->data(SceneRole).toString())).arg(esc(r->data(TargetSceneRole).toString())).arg(esc(r->data(ShapeRole).toString()));
      ents+=QString("scene.e.back().z=%1;scene.e.back().rotation=%2;scene.e.back().scale=%3;scene.e.back().opacity=%4;scene.e.back().visible=%5;scene.e.back().blur=%6;scene.e.back().physics=%7;scene.e.back().glass=%8;scene.e.back().textFollowsOpacity=%9;\n").arg(r->data(ZRole).toInt()).arg(r->data(RotationRole).toDouble()).arg(r->data(ScaleRole).toDouble()).arg(r->data(OpacityRole).toDouble()).arg(r->data(ObjectVisibleRole).toBool()?"true":"false").arg(r->data(BlurRole).toDouble()).arg(r->data(PhysicsRole).toBool()?"true":"false").arg(r->data(GlassRole).toBool()?"true":"false").arg(r->data(TextOpacityRole).toBool()?"true":"false");
      QVariantMap painted=r->data(CellColorsRole).toMap();for(auto it=painted.begin();it!=painted.end();++it){QColor pc=it.value().value<QColor>();ents+=QString("scene.e.back().cellColors[\"%1\"]=QColor(%2,%3,%4,%5);\n").arg(esc(it.key())).arg(pc.red()).arg(pc.green()).arg(pc.blue()).arg(pc.alpha());}
      QVariantMap placedTiles=r->data(TileCellsRole).toMap();for(auto it=placedTiles.begin();it!=placedTiles.end();++it)ents+=QString("scene.e.back().tileCells[\"%1\"]=%2;\n").arg(esc(it.key())).arg(it.value().toInt());
      ents+=QString("scene.e.back().uiState=%1;\n").arg(r->data(UiStateRole).toBool()?"true":"false");
      ents+=QString("scene.e.back().targetObject=\"%1\";scene.e.back().altImage=\"%2\";\n").arg(esc(r->data(TargetObjectRole).toString()),esc(altAssetPath));
      ents+=QString("scene.e.back().cornerRadius=%1;scene.e.back().uiValue=%2;\n").arg(r->data(CornerRadiusRole).toDouble()).arg(r->data(UiValueRole).toInt());
      QColor press=r->data(PressColorRole).value<QColor>();ents+=QString("scene.e.back().pressColor=QColor(%1,%2,%3,%4);scene.e.back().pressEffect=%5;\n").arg(press.red()).arg(press.green()).arg(press.blue()).arg(press.alpha()).arg(r->data(PressEffectRole).toBool()?"true":"false");
      ents+=QString("scene.e.back().autoPlay=%1;scene.e.back().loop=%2;scene.e.back().volume=%3;scene.e.back().saveKey=\"%4\";scene.e.back().saveFields=\"%5\";scene.e.back().autoLoad=%6;scene.e.back().vibrationDuration=%7;scene.e.back().vibrationOnStart=%8;scene.e.back().radioGroup=\"%9\";\n").arg(r->data(AutoPlayRole).toBool()?"true":"false").arg(r->data(LoopRole).toBool()?"true":"false").arg(r->data(VolumeRole).toInt()).arg(esc(r->data(SaveKeyRole).toString())).arg(esc(r->data(SaveFieldsRole).toString())).arg(r->data(AutoLoadRole).toBool()?"true":"false").arg(r->data(VibrationDurationRole).toInt()).arg(r->data(VibrationOnStartRole).toBool()?"true":"false").arg(esc(r->data(RadioGroupRole).toString()));
    }
    QString componentCalls;for(auto it=scriptTargets.begin();it!=scriptTargets.end();++it){QString safe=it.value();safe.replace(QRegularExpression("[^A-Za-z0-9_]"),"_");componentCalls+=QString("if(auto* component=scene.find(\"%1\")) update_%2(scene,*component,dt);\n").arg(esc(it.value()),safe);}QString launchCode=androidFullscreen?QString("g.showFullScreen();"):QString("g.show();g.applyAndroidBars(QColor(%1,%2,%3,%4));").arg(androidSystemBarColor.red()).arg(androidSystemBarColor.green()).arg(androidSystemBarColor.blue()).arg(androidSystemBarColor.alpha());QString runtimeIcon=gameIconPath;if(android&&!runtimeIcon.isEmpty())runtimeIcon=":/assets/"+QFileInfo(runtimeIcon).fileName();
    return QString(R"CPP(#include <QtWidgets>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QAudioOutput>
#include <QtMultimedia/QVideoSink>
#include <QtMultimedia/QVideoFrame>
#include <QtMultimediaWidgets/QVideoWidget>
#ifdef Q_OS_WIN
#include <windows.h>
#endif
#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QtCore/qnativeinterface.h>
#endif
#include <cmath>
#include <algorithm>
struct Input{bool left=false,right=false,up=false,down=false,space=false;float mouseX=0,mouseY=0;bool mouseDown=false,mousePressed=false,mouseReleased=false;};
struct Entity{QString name,type;float x,y,velocityX,velocityY,rotation;QColor color;bool visible;QString behavior,image,fit;float width=60,height=60;QString font="Segoe UI";int cellSize=32,columns=25,rows=15;bool gridVisible=true;QColor fill,stroke;float gravity=0;bool lockRotation=false;QString cameraFollow="None";bool cameraAdaptive=true;int cullMargin=64;QString up="W",down="S",left="A",right="D";bool hitbox=true,showHitbox=false;QString text,sceneName="Scene 1",targetScene="Scene 1",shape="Circle",targetObject,altImage;QColor pressColor=QColor("#a997ff");int z=0,uiValue=50;float scale=1,opacity=1,blur=0,cornerRadius=10,pressUntil=0;bool physics=false,glass=false,textFollowsOpacity=true,uiState=false,pressEffect=true;QHash<QString,QColor>cellColors;QHash<QString,int>tileCells;bool autoPlay=true,loop=false,autoLoad=true,vibrationOnStart=false;int volume=80,vibrationDuration=120;QString saveKey="MainSave",saveFields,radioGroup="Default";};
struct Scene{QVector<Entity>e;Input input;float time=0;std::function<void(int)>vibration;std::function<void(const QString&)>saveCallback,loadCallback;Entity*find(const QString&n){for(auto&i:e)if(i.name==n)return&i;return nullptr;}Entity*findType(const QString&t){for(auto&i:e)if(i.type==t)return&i;return nullptr;}void vibrate(int ms=120){if(vibration)vibration(ms);}void saveGame(const QString&slot="MainSave"){if(saveCallback)saveCallback(slot);}void loadGame(const QString&slot="MainSave"){if(loadCallback)loadCallback(slot);}};
%1
class Game:public QWidget{
 Scene scene; QElapsedTimer timer;QString activeScene="%4";float cameraX=0,cameraY=0;bool pointerMove=false,servicesReady=false;bool portrait=%6;int gamePixelWidth=%7,gamePixelHeight=%8;QPointF pointerTarget;QHash<QString,QMediaPlayer*>mediaPlayers;QHash<QString,QVideoWidget*>videoWidgets;
 void showEvent(QShowEvent*event)override{QWidget::showEvent(event);static bool sized=false;if(!sized){sized=true;resize(gamePixelWidth,gamePixelHeight);setWindowTitle("%9");QString iconPath="%10";if(!iconPath.isEmpty())setWindowIcon(QIcon(iconPath));}setupRuntimeServices();}
 void closeEvent(QCloseEvent*event)override{for(auto&i:scene.e)if(i.type=="Save Data")saveSlot(i.saveKey);QWidget::closeEvent(event);}
 bool event(QEvent*event)override{if(event->type()==QEvent::TouchBegin||event->type()==QEvent::TouchUpdate||event->type()==QEvent::TouchEnd||event->type()==QEvent::TouchCancel){auto*touch=static_cast<QTouchEvent*>(event);if(!touch->points().isEmpty()){QPointF world=screenToWorld(touch->points().first().position());scene.input.mouseX=world.x();scene.input.mouseY=world.y();pointerTarget=world;}if(event->type()==QEvent::TouchBegin){scene.input.mouseDown=true;scene.input.mousePressed=true;pointerMove=true;}else if(event->type()==QEvent::TouchEnd||event->type()==QEvent::TouchCancel){scene.input.mouseDown=false;scene.input.mouseReleased=true;scene.input.mousePressed=false;pointerMove=false;}event->accept();return true;}return QWidget::event(event);}
 void material(QPainter&p,const Entity&i,const QRectF&box){if(i.glass){p.setPen(QPen(QColor(255,255,255,130),1.5));p.setBrush(QColor(220,235,255,38));p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);p.setPen(QPen(QColor(255,255,255,75),2));p.drawLine(box.topLeft()+QPointF(i.cornerRadius+6,7),box.topRight()+QPointF(-i.cornerRadius-6,7));}if(i.blur>0&&i.image.isEmpty()){QColor glow=i.color;glow.setAlpha(qBound(12,qRound(i.blur*.7f),70));p.setBrush(Qt::NoBrush);for(int k=1;k<=3;k++){p.setPen(QPen(glow,k*2));p.drawRoundedRect(box.adjusted(-k,-k,k,k),i.cornerRadius+k,i.cornerRadius+k);}}}
 QUrl playableUrl(const QString&path){if(!path.startsWith(":/"))return QUrl::fromLocalFile(path);QFile source(path);if(!source.open(QIODevice::ReadOnly))return {};QString folder=QStandardPaths::writableLocation(QStandardPaths::CacheLocation);QDir().mkpath(folder);QString target=folder+"/"+QFileInfo(path).fileName();QFile out(target);if(out.open(QIODevice::WriteOnly)){out.write(source.readAll());out.close();}return QUrl::fromLocalFile(target);}
 QVariant entityProperty(Entity&e,const QString&p){if(p=="x")return e.x;if(p=="y")return e.y;if(p=="rotation")return e.rotation;if(p=="visible")return e.visible;if(p=="text")return e.text;if(p=="uiValue")return e.uiValue;if(p=="uiState")return e.uiState;if(p=="color")return e.color;return {};}
 void setEntityProperty(Entity&e,const QString&p,const QVariant&v){if(p=="x")e.x=v.toFloat();else if(p=="y")e.y=v.toFloat();else if(p=="rotation")e.rotation=v.toFloat();else if(p=="visible")e.visible=v.toBool();else if(p=="text")e.text=v.toString();else if(p=="uiValue")e.uiValue=v.toInt();else if(p=="uiState")e.uiState=v.toBool();else if(p=="color")e.color=v.value<QColor>();}
 Entity*saveBlock(const QString&slot){for(auto&i:scene.e)if(i.type=="Save Data"&&(i.saveKey==slot||slot.isEmpty()))return&i;return nullptr;}
 void saveSlot(const QString&slot){Entity*block=saveBlock(slot);if(!block)return;QSettings settings("EnginePlusPlus","%9");settings.beginGroup(block->saveKey);for(const QString&token:block->saveFields.split(',',Qt::SkipEmptyParts)){QStringList pair=token.trimmed().split('.');if(pair.size()!=2)continue;if(Entity*target=scene.find(pair[0]))settings.setValue(token.trimmed(),entityProperty(*target,pair[1]));}settings.endGroup();settings.sync();}
 void loadSlot(const QString&slot){Entity*block=saveBlock(slot);if(!block)return;QSettings settings("EnginePlusPlus","%9");settings.beginGroup(block->saveKey);for(const QString&token:block->saveFields.split(',',Qt::SkipEmptyParts)){QStringList pair=token.trimmed().split('.');if(pair.size()!=2||!settings.contains(token.trimmed()))continue;if(Entity*target=scene.find(pair[0]))setEntityProperty(*target,pair[1],settings.value(token.trimmed()));}settings.endGroup();update();}
 bool eventFilter(QObject*object,QEvent*event)override{Q_UNUSED(object);QPointF position;bool pressed=false;if(event->type()==QEvent::MouseButtonPress){auto*mouse=static_cast<QMouseEvent*>(event);if(mouse->button()==Qt::LeftButton){position=mouse->position();pressed=true;}}else if(event->type()==QEvent::TouchBegin){auto*touch=static_cast<QTouchEvent*>(event);if(!touch->points().isEmpty()){position=touch->points().first().position();pressed=true;}}if(pressed){QPointF world=screenToWorld(position);for(auto&i:scene.e)if(i.type=="Radio Button"&&i.sceneName==activeScene&&QRectF(i.x-i.width/2,i.y-i.height/2,i.width,i.height).contains(world)){for(auto&other:scene.e)if(other.type=="Radio Button"&&other.sceneName==i.sceneName&&other.radioGroup==i.radioGroup)other.uiState=false;i.uiState=true;update();return true;}}return false;}
 void setupRuntimeServices(){if(servicesReady)return;servicesReady=true;installEventFilter(this);scene.saveCallback=[this](const QString&s){saveSlot(s);};scene.loadCallback=[this](const QString&s){loadSlot(s);};scene.vibration=[this](int ms){
#ifdef Q_OS_ANDROID
 QJniObject context=QNativeInterface::QAndroidApplication::context();QJniObject name=QJniObject::fromString("vibrator");QJniObject vibrator=context.callObjectMethod("getSystemService","(Ljava/lang/String;)Ljava/lang/Object;",name.object<jstring>());if(vibrator.isValid())vibrator.callMethod<void>("vibrate","(J)V",(jlong)qMax(1,ms));
#else
 Q_UNUSED(ms);QApplication::beep();
#endif
 };for(auto&i:scene.e){if(i.type=="Video Player"||i.type=="Audio Source"){auto*player=new QMediaPlayer(this);auto*audio=new QAudioOutput(player);audio->setVolume(qBound(0,i.volume,100)/100.0);player->setAudioOutput(audio);player->setLoops(i.loop?QMediaPlayer::Infinite:1);if(i.type=="Video Player"){auto*video=new QVideoWidget(this);video->setAspectRatioMode(i.fit=="Fill"?Qt::IgnoreAspectRatio:(i.fit=="Crop"?Qt::KeepAspectRatioByExpanding:Qt::KeepAspectRatio));video->show();player->setVideoOutput(video);videoWidgets[i.name]=video;}player->setSource(playableUrl(i.image));mediaPlayers[i.name]=player;if(i.autoPlay)player->play();}if(i.type=="Save Data"&&i.autoLoad)loadSlot(i.saveKey);if(i.type=="Vibration"&&i.vibrationOnStart)scene.vibrate(i.vibrationDuration);}auto*layoutTimer=new QTimer(this);connect(layoutTimer,&QTimer::timeout,this,[this]{float baseW=portrait?450.f:800.f,baseH=portrait?800.f:450.f;float scale=qMin(width()/baseW,height()/baseH);for(auto&i:scene.e)if(i.type=="Video Player"&&videoWidgets.contains(i.name)){QVideoWidget*v=videoWidgets[i.name];v->setVisible(i.visible&&i.sceneName==activeScene);v->setGeometry(qRound(width()/2+(i.x-i.width/2-cameraX)*scale),qRound(height()/2+(i.y-i.height/2-cameraY)*scale),qMax(1,qRound(i.width*scale)),qMax(1,qRound(i.height*scale)));v->raise();}});layoutTimer->start(33);}
public:
 void applyAndroidBars(const QColor&color){
#ifdef Q_OS_ANDROID
  QJniObject activity=QNativeInterface::QAndroidApplication::context();QJniObject window=activity.callObjectMethod("getWindow","()Landroid/view/Window;");if(window.isValid()){jint value=(jint)color.rgba();window.callMethod<void>("setStatusBarColor","(I)V",value);window.callMethod<void>("setNavigationBarColor","(I)V",value);}
#else
  Q_UNUSED(color);
#endif
 }
 Game(){setWindowTitle("Engine++ Game Preview");resize(1280,720);setFocusPolicy(Qt::StrongFocus);setMouseTracking(true);setAttribute(Qt::WA_AcceptTouchEvents,true);%2 std::stable_sort(scene.e.begin(),scene.e.end(),[](const Entity&a,const Entity&b){return a.z<b.z;});onStart(scene);timer.start();auto*t=new QTimer(this);connect(t,&QTimer::timeout,this,[this]{float dt=qMin(timer.restart()/1000.f,.05f);scene.time+=dt;for(auto&i:scene.e){if(i.behavior=="Rotator"&&!i.lockRotation)i.rotation+=90*dt;if(i.behavior=="Floating")i.y+=std::sin(scene.time*3)*20*dt;if(i.behavior=="Patrol")i.x+=std::cos(scene.time)*100*dt;if(i.behavior=="Pulse Shader")i.color.setAlphaF(.55+.45*std::sin(scene.time*4));if(i.behavior=="Bounce"){i.velocityY+=500*dt;i.y+=i.velocityY*dt;if(i.y>220){i.y=220;i.velocityY=-320;}}if(i.behavior=="Orbit"){i.rotation+=70*dt;i.x+=std::cos(scene.time*2)*45*dt;i.y+=std::sin(scene.time*2)*45*dt;}if(i.behavior=="Blink")i.opacity=std::fmod(scene.time,1.f)<.65f?1.f:.12f;if(i.behavior=="Pulse Scale")i.scale=1.f+.14f*std::sin(scene.time*5);if(i.type=="Character"&&i.behavior=="Player Controller"&&pointerMove){QPointF d=pointerTarget-QPointF(i.x,i.y);if(QLineF(QPointF(),d).length()>5){i.x+=qBound(-240.f,(float)d.x()*4.f,240.f)*dt;i.y+=qBound(-240.f,(float)d.y()*4.f,240.f)*dt;}}if(i.type=="Character"||i.physics){i.velocityY+=i.gravity*dt;i.y+=i.velocityY*dt;if(i.lockRotation)i.rotation=0;if(i.hitbox)for(auto&g:scene.e)if(&g!=&i&&g.hitbox&&(g.type=="Solid Ground"||g.type=="Platform"||g.type=="Collider")){QRectF body(i.x-i.width/2,i.y-i.height/2,i.width,i.height),ground(g.x-g.width/2,g.y-g.height/2,g.width,g.height);if(body.intersects(ground)&&i.velocityY>=0){i.y=ground.top()-i.height/2;i.velocityY=0;}}}}onUpdate(scene,dt);%3 update();});t->start(16);}
protected:
 void paintEvent(QPaintEvent*)override{QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.fillRect(rect(),QColor("#0b0f18"));int baseW=portrait?450:800,baseH=portrait?800:450;Entity*cam=nullptr;for(auto&i:scene.e)if(i.type=="Camera"&&i.sceneName==activeScene){cam=&i;break;}float cx=cam?cam->x:0,cy=cam?cam->y:0;if(cam){baseW=qMax(1,qRound(cam->width));baseH=qMax(1,qRound(cam->height));}if(cam&&cam->cameraFollow!="None")if(auto*t=scene.find(cam->cameraFollow)){cx=t->x;cy=t->y;}cameraX=cx;cameraY=cy;p.translate(width()/2,height()/2);if(!cam||cam->cameraAdaptive){float uniformScale=qMin(width()/float(baseW),height()/float(baseH));p.scale(uniformScale,uniformScale);}p.translate(-cx,-cy);QRectF visible(cx-baseW/2-(cam?cam->cullMargin:64),cy-baseH/2-(cam?cam->cullMargin:64),baseW+2*(cam?cam->cullMargin:64),baseH+2*(cam?cam->cullMargin:64));for(auto&i:scene.e){if(i.sceneName!=activeScene||!i.visible||i.type=="Camera"||i.type=="Audio Source"||i.type=="Video Player")continue;if(cam&&cam->cameraAdaptive&&!visible.intersects(QRectF(i.x-i.width/2,i.y-i.height/2,i.width,i.height)))continue;p.save();p.translate(i.x,i.y);p.rotate(i.rotation);p.scale(i.scale,i.scale);p.setOpacity(i.opacity);QRectF box(-i.width/2,-i.height/2,i.width,i.height);p.setPen(QPen(i.color.lighter(150),2));p.setBrush(i.color);
   p.setRenderHint(QPainter::SmoothPixmapTransform);p.setRenderHint(QPainter::LosslessImageRendering);if(!i.image.isEmpty()&&i.type!="Tile Map"&&i.type!="Theme Switch"){QPixmap src(i.image);if(!src.isNull()){if(i.blur>0){QSize original=src.size();int divisor=qMax(2,1+qRound(i.blur/12));src=src.scaled(qMax(1,src.width()/divisor),qMax(1,src.height()/divisor),Qt::IgnoreAspectRatio,Qt::SmoothTransformation).scaled(original,Qt::IgnoreAspectRatio,Qt::SmoothTransformation);}p.save();QPainterPath clip;clip.addRoundedRect(box,i.cornerRadius,i.cornerRadius);p.setClipPath(clip);if(i.fit=="Center")p.drawPixmap(QPointF(-src.width()/2,-src.height()/2),src);else{Qt::AspectRatioMode mode=i.fit=="Fill"?Qt::IgnoreAspectRatio:(i.fit=="Crop"?Qt::KeepAspectRatioByExpanding:Qt::KeepAspectRatio);QPixmap scaled=src.scaled(box.size().toSize()*devicePixelRatioF(),mode,Qt::SmoothTransformation);scaled.setDevicePixelRatio(devicePixelRatioF());p.drawPixmap(QPointF(-scaled.deviceIndependentSize().width()/2,-scaled.deviceIndependentSize().height()/2),scaled);}p.restore();material(p,i,box);}}
   else if(i.type=="Tile Map"){QPixmap tileset(i.image);QRectF area(-i.width/2,-i.height/2,i.width,i.height);p.fillRect(area,i.fill);int sourceCols=tileset.isNull()?1:qMax(1,tileset.width()/i.cellSize);for(auto it=i.tileCells.begin();it!=i.tileCells.end();++it){QStringList xy=it.key().split(',');if(xy.size()!=2||tileset.isNull())continue;int index=it.value(),sx=(index%sourceCols)*i.cellSize,sy=(index/sourceCols)*i.cellSize;p.drawPixmap(QRectF(area.left()+xy[0].toInt()*i.cellSize,area.top()+xy[1].toInt()*i.cellSize,i.cellSize,i.cellSize),tileset,QRectF(sx,sy,i.cellSize,i.cellSize));}if(i.gridVisible){p.setPen(QPen(i.stroke,1));for(int x=0;x<=i.columns;x++)p.drawLine(area.left()+x*i.cellSize,area.top(),area.left()+x*i.cellSize,area.top()+i.rows*i.cellSize);for(int y=0;y<=i.rows;y++)p.drawLine(area.left(),area.top()+y*i.cellSize,area.left()+i.columns*i.cellSize,area.top()+y*i.cellSize);}}
   else if(i.type=="Grid Map"){p.fillRect(QRectF(0,0,i.columns*i.cellSize,i.rows*i.cellSize),i.fill);for(auto it=i.cellColors.begin();it!=i.cellColors.end();++it){QStringList xy=it.key().split(',');if(xy.size()==2)p.fillRect(QRectF(xy[0].toInt()*i.cellSize,xy[1].toInt()*i.cellSize,i.cellSize,i.cellSize),it.value());}if(i.gridVisible){p.setPen(QPen(i.stroke,1));for(int x=0;x<=i.columns;x++)p.drawLine(x*i.cellSize,0,x*i.cellSize,i.rows*i.cellSize);for(int y=0;y<=i.rows;y++)p.drawLine(0,y*i.cellSize,i.columns*i.cellSize,y*i.cellSize);}}
   else if(i.type=="ASFO Territory"){p.setBrush(i.fill.isValid()?i.fill:i.color);if(i.shape=="Square")p.drawRect(box);else if(i.shape=="Rounded Square")p.drawRoundedRect(box,40,40);else if(i.shape=="Hexagon"){QPolygonF hex;for(int n=0;n<6;n++){double a=n*M_PI/3;hex<<QPointF(std::cos(a)*i.width/2,std::sin(a)*i.height/2);}p.drawPolygon(hex);}else p.drawEllipse(box);}
   else if(i.type=="Theme Switch"){p.setPen(QPen(i.uiState?QColor("#7f8bad"):QColor("#d7dcef"),2));p.setBrush(i.uiState?QColor("#171c2b"):QColor("#f5f7ff"));p.drawRoundedRect(box,box.height()/2,box.height()/2);p.setPen(i.uiState?Qt::white:QColor("#22293a"));p.drawText(box,Qt::AlignCenter,i.uiState?"DARK":"LIGHT");}
   else if(i.type=="Text"){if(i.glass||i.blur>0){p.setBrush(QColor(20,27,43,90));p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);material(p,i,box);}p.setPen(i.color);p.setFont(QFont(i.font,20,QFont::Bold));p.drawText(box,Qt::AlignCenter,i.text.isEmpty()?i.name:i.text);}
   else if(i.type=="Character"||i.type=="Platform"||i.type=="Physics Block"||i.type=="Solid Ground"||i.type=="Collider"||i.type=="Sprite"){p.setPen(QPen(i.color.lighter(150),2));p.setBrush(i.color.darker(165));p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);material(p,i,box);}
   else if(i.type=="Slider"||i.type=="Progress Bar"){int value=qBound(0,i.uiValue,100);p.setPen(Qt::NoPen);p.setBrush(QColor("#303a52"));p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);QRectF fill=box;fill.setWidth(box.width()*value/100.0);p.setBrush(i.color);p.drawRoundedRect(fill,i.cornerRadius,i.cornerRadius);if(i.type=="Slider"){p.setBrush(Qt::white);p.drawEllipse(QPointF(box.left()+box.width()*value/100.0,box.center().y()),qMax(7.0,box.height()*.32),qMax(7.0,box.height()*.32));}else{p.setPen(Qt::white);p.drawText(box,Qt::AlignCenter,QString::number(value)+"%");}material(p,i,box);}
   else if(i.type=="Check Box"){QRectF check(box.left()+5,box.center().y()-13,26,26);p.setBrush(i.uiState?i.color:QColor("#242c40"));p.setPen(QPen(i.color.lighter(145),2));p.drawRoundedRect(check,6,6);if(i.uiState){p.setPen(QPen(Qt::white,3));p.drawLine(check.left()+6,check.center().y(),check.center().x()-1,check.bottom()-6);p.drawLine(check.center().x()-1,check.bottom()-6,check.right()-5,check.top()+6);}p.setPen(Qt::white);p.drawText(box.adjusted(40,0,0,0),Qt::AlignVCenter|Qt::AlignLeft,i.text);material(p,i,box);}
   else if(i.type=="Radio Button"){QRectF radio(box.left()+7,box.center().y()-12,24,24);p.setPen(QPen(i.uiState?i.color:QColor("#66718c"),2));p.setBrush(QColor("#151d30"));p.drawEllipse(radio);if(i.uiState){p.setPen(Qt::NoPen);p.setBrush(i.color);p.drawEllipse(radio.adjusted(6,6,-6,-6));}p.setPen(Qt::white);p.drawText(box.adjusted(42,0,0,0),Qt::AlignVCenter|Qt::AlignLeft,i.text);material(p,i,box);}
   else if(i.type=="Button Container"||i.type=="UI Container"){QStringList labels=i.text.split(',',Qt::SkipEmptyParts);if(labels.isEmpty())labels={"Play","Settings","Exit"};bool vertical=i.shape!="Horizontal";p.setBrush(QColor(18,25,42,210));p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);for(int n=0;n<labels.size();++n){QRectF b=vertical?QRectF(box.left()+10,box.top()+10+n*(box.height()-20)/labels.size(),box.width()-20,(box.height()-20)/labels.size()-6):QRectF(box.left()+10+n*(box.width()-20)/labels.size(),box.top()+10,(box.width()-20)/labels.size()-6,box.height()-20);p.setBrush(i.color.darker(145));p.drawRoundedRect(b,qMin(i.cornerRadius,8.f),qMin(i.cornerRadius,8.f));p.setPen(Qt::white);p.drawText(b,Qt::AlignCenter,labels[n].trimmed());}material(p,i,box);}
   else if(i.type=="Scroll View"){p.setPen(QPen(i.color.lighter(130),2));p.setBrush(QColor("#111827"));p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);p.save();p.setClipRect(box.adjusted(8,8,-14,-8));QStringList items=i.text.split(',',Qt::SkipEmptyParts);p.setPen(Qt::white);float offset=i.uiValue*qMax(0,items.size()*30-qRound(box.height()-20))/100.f;for(int n=0;n<items.size();++n)p.drawText(QRectF(box.left()+14,box.top()+12+n*30-offset,box.width()-34,26),Qt::AlignVCenter|Qt::AlignLeft,items[n].trimmed());p.restore();p.setBrush(QColor("#596780"));p.setPen(Qt::NoPen);float travel=qMax(0.0,box.height()-50.0);p.drawRoundedRect(QRectF(box.right()-9,box.top()+10+travel*i.uiValue/100.0,4,30),2,2);material(p,i,box);}
   else if(i.type=="Dropdown"){p.setPen(QPen(i.color.lighter(145),2));p.setBrush(i.color.darker(165));p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);p.setPen(Qt::white);p.drawText(box.adjusted(12,0,-34,0),Qt::AlignVCenter|Qt::AlignLeft,i.text.split(',',Qt::SkipEmptyParts).value(0,"Select").trimmed());p.drawText(box.adjusted(box.width()-34,0,-8,0),Qt::AlignCenter,"▼");material(p,i,box);}
   else if(i.type=="Button"){QColor buttonColor=i.pressEffect&&scene.time<i.pressUntil?i.pressColor:i.color;p.setPen(QPen(buttonColor.lighter(145),2));p.setBrush(buttonColor.darker(155));p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);material(p,i,box);if(!i.textFollowsOpacity)p.setOpacity(1.0);p.setPen(Qt::white);p.setFont(QFont(i.font,16,QFont::Bold));p.drawText(box,Qt::AlignCenter,i.text.isEmpty()?i.name:i.text);}
   else if(i.type=="Toggle"){p.setPen(Qt::NoPen);p.setBrush(i.uiState?i.color:QColor("#3a4358"));p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);qreal d=box.height()-10;p.setBrush(Qt::white);p.drawEllipse(QRectF(i.uiState?box.right()-d-5:box.left()+5,box.top()+5,d,d));p.setPen(Qt::white);p.drawText(box.adjusted(8,0,-8,0),i.uiState?Qt::AlignLeft|Qt::AlignVCenter:Qt::AlignRight|Qt::AlignVCenter,i.uiState?"ON":"OFF");material(p,i,box);}
   else if(i.type=="Panel"||i.type=="Glass Panel"||i.type=="Color Block"){p.setPen(QPen(i.color.lighter(140),2));p.setBrush(i.color);p.drawRoundedRect(box,i.cornerRadius,i.cornerRadius);material(p,i,box);}
   else if(i.type=="Color Block"){p.setPen(i.blur>0?Qt::NoPen:QPen(i.color.lighter(140),2));if(i.blur>0){QRadialGradient soft(0,0,qMax(i.width,i.height)*.72);QColor edge=i.color;edge.setAlpha(0);soft.setColorAt(0,i.color);soft.setColorAt(qBound(.05f,1.f-i.blur/110.f,.95f),i.color);soft.setColorAt(1,edge);p.setBrush(soft);}p.drawRoundedRect(box,12,12);}else if(i.type=="Panel"||i.type=="Glass Panel"){bool glass=i.glass||i.type=="Glass Panel";QColor panel=i.color;panel.setAlpha(glass?72:210);QLinearGradient g(box.topLeft(),box.bottomRight());g.setColorAt(0,panel.lighter(145));g.setColorAt(1,panel.darker(125));p.setBrush(g);p.setPen(QPen(glass?QColor(255,255,255,130):i.color.lighter(135),glass?2:1));p.drawRoundedRect(box,18,18);if(glass){p.setPen(QPen(QColor(255,255,255,85),2));p.drawLine(box.topLeft()+QPointF(18,10),box.topRight()+QPointF(-18,10));}}else if(i.type=="Physics Block"){p.setPen(QPen(i.color.lighter(155),3));p.setBrush(i.color.darker(155));p.drawRoundedRect(box,7,7);p.drawLine(box.topLeft(),box.bottomRight());p.drawLine(box.topRight(),box.bottomLeft());}else if(i.type=="Character")p.drawRoundedRect(box,10,10);else if(i.type=="Light"){QRadialGradient light(0,0,qMax(i.width,i.height)/2);QColor core=i.color;core.setAlpha(210);QColor edge=i.color;edge.setAlpha(0);light.setColorAt(0,core);light.setColorAt(.35,QColor(core.red(),core.green(),core.blue(),110));light.setColorAt(1,edge);p.setCompositionMode(QPainter::CompositionMode_Screen);p.setPen(Qt::NoPen);p.setBrush(light);p.drawEllipse(box);p.setCompositionMode(QPainter::CompositionMode_SourceOver);}else if(i.type=="Particle"){for(int k=0;k<12;k++){float a=k*.52+scene.time;p.drawEllipse(QPointF(std::cos(a)*35,std::sin(a)*35),3,3);}}else if(i.type=="Button Container"||i.type=="UI Container"){QStringList labels=i.text.split(',',Qt::SkipEmptyParts);if(labels.isEmpty())labels={"Play","Settings","Exit"};bool vertical=i.shape!="Horizontal";p.setBrush(QColor(18,25,42,210));p.drawRoundedRect(box,14,14);for(int n=0;n<labels.size();++n){QRectF b=vertical?QRectF(box.left()+10,box.top()+10+n*(box.height()-20)/labels.size(),box.width()-20,(box.height()-20)/labels.size()-6):QRectF(box.left()+10+n*(box.width()-20)/labels.size(),box.top()+10,(box.width()-20)/labels.size()-6,box.height()-20);p.setBrush(i.color.darker(145));p.drawRoundedRect(b,8,8);p.setPen(Qt::white);p.drawText(b,Qt::AlignCenter,labels[n].trimmed());}}else if(i.type=="Dropdown"){p.setBrush(i.color.darker(165));p.drawRoundedRect(box,9,9);p.setPen(Qt::white);p.drawText(box.adjusted(12,0,-34,0),Qt::AlignVCenter|Qt::AlignLeft,i.text.split(',',Qt::SkipEmptyParts).value(0,"Select").trimmed());p.drawText(box.adjusted(box.width()-34,0,-8,0),Qt::AlignCenter,"▼");}else if(i.type=="Toggle"){p.setPen(Qt::NoPen);p.setBrush(i.uiState?QColor("#28d7b8"):QColor("#3a4358"));p.drawRoundedRect(box,box.height()/2,box.height()/2);qreal d=box.height()-10;p.setBrush(Qt::white);p.drawEllipse(QRectF(i.uiState?box.right()-d-5:box.left()+5,box.top()+5,d,d));p.setPen(Qt::white);p.drawText(box.adjusted(8,0,-8,0),i.uiState?Qt::AlignLeft|Qt::AlignVCenter:Qt::AlignRight|Qt::AlignVCenter,i.uiState?"ON":"OFF");}else if(i.type=="Button"){p.setPen(QPen(i.color.lighter(145),2));p.setBrush(i.color.darker(155));p.drawRoundedRect(box,10,10);if(!i.textFollowsOpacity)p.setOpacity(1.0);p.setPen(Qt::white);p.setFont(QFont(i.font,16,QFont::Bold));p.drawText(box,Qt::AlignCenter,i.text.isEmpty()?i.name:i.text);}else if(i.type=="Text"){p.setPen(i.color);p.setFont(QFont(i.font,20,QFont::Bold));p.drawText(box,Qt::AlignCenter,i.text.isEmpty()?i.name:i.text);}else p.drawRect(box);if(i.showHitbox){p.setBrush(Qt::NoBrush);p.setPen(QPen(QColor("#ff3b64"),2,Qt::DashLine));p.drawRect(box);}p.setPen(Qt::white);if(i.type!="Text"&&i.type!="Button"&&i.type!="Button Container"&&i.type!="UI Container"&&i.type!="Dropdown"&&i.type!="Toggle"&&i.type!="Background Image")p.drawText(QRectF(-80,i.height/2+6,160,24),Qt::AlignCenter,i.name);p.restore();}}
 QPointF screenToWorld(const QPointF&pos){float baseW=portrait?450.f:800.f,baseH=portrait?800.f:450.f;for(auto&i:scene.e)if(i.type=="Camera"&&i.sceneName==activeScene){baseW=i.width;baseH=i.height;break;}float uniformScale=qMax(.0001f,qMin(width()/baseW,height()/baseH));return QPointF((pos.x()-width()/2)/uniformScale+cameraX,(pos.y()-height()/2)/uniformScale+cameraY);}void syncMouse(const QPointF&world){scene.input.mouseX=world.x();scene.input.mouseY=world.y();}void mousePressEvent(QMouseEvent*e)override{QPointF world=screenToWorld(e->position());syncMouse(world);if(e->button()==Qt::LeftButton){scene.input.mouseDown=true;scene.input.mousePressed=true;}for(auto&i:scene.e){QRectF hit(i.x-i.width/2,i.y-i.height/2,i.width,i.height);if(i.sceneName!=activeScene||!hit.contains(world))continue;if(i.pressEffect)i.pressUntil=scene.time+.18f;if(i.type=="Toggle"||i.type=="Check Box"){i.uiState=!i.uiState;update();return;}if(i.type=="Slider"){i.uiValue=qBound(0,qRound((world.x()-hit.left())/hit.width()*100),100);update();return;}if(i.type=="Theme Switch"){i.uiState=!i.uiState;for(const QString&name:i.targetObject.split(',',Qt::SkipEmptyParts))if(auto*target=scene.find(name.trimmed())){target->color=i.uiState?i.fill:i.color;QString next=i.uiState?i.altImage:i.image;if(!next.isEmpty())target->image=next;}update();return;}if(i.type=="Dropdown"){QStringList options=i.text.split(',',Qt::SkipEmptyParts);if(options.size()>1){QString first=options.takeFirst();options.append(first);i.text=options.join(',');}update();return;}if(i.type=="Button"||i.type=="Scene Portal"){activeScene=i.targetScene;update();return;}}pointerTarget=world;pointerMove=true;}void mouseMoveEvent(QMouseEvent*e)override{QPointF world=screenToWorld(e->position());syncMouse(world);if(pointerMove)pointerTarget=world;}void mouseReleaseEvent(QMouseEvent*e)override{syncMouse(screenToWorld(e->position()));if(e->button()==Qt::LeftButton){scene.input.mouseDown=false;scene.input.mouseReleased=true;scene.input.mousePressed=false;}pointerMove=false;}void wheelEvent(QWheelEvent*e)override{QPointF world=screenToWorld(e->position());for(auto&i:scene.e)if(i.sceneName==activeScene&&i.type=="Scroll View"&&QRectF(i.x-i.width/2,i.y-i.height/2,i.width,i.height).contains(world)){i.uiValue=qBound(0,i.uiValue+(e->angleDelta().y()<0?10:-10),100);update();e->accept();return;}QWidget::wheelEvent(e);}
 void keyPressEvent(QKeyEvent*e)override{key(e,true);}void keyReleaseEvent(QKeyEvent*e)override{key(e,false);}int mapped(const QString&s,int fallback){QKeySequence q(s);return q.isEmpty()?fallback:q[0].key();}void key(QKeyEvent*e,bool v){Entity*c=scene.findType("Character");if(e->key()==Qt::Key_Left||e->key()==mapped(c?c->left:"A",Qt::Key_A))scene.input.left=v;if(e->key()==Qt::Key_Right||e->key()==mapped(c?c->right:"D",Qt::Key_D))scene.input.right=v;if(e->key()==Qt::Key_Up||e->key()==mapped(c?c->up:"W",Qt::Key_W))scene.input.up=v;if(e->key()==Qt::Key_Down||e->key()==mapped(c?c->down:"S",Qt::Key_S))scene.input.down=v;if(e->key()==Qt::Key_Space)scene.input.space=v;}
};
#ifdef Q_OS_WIN
int WINAPI wWinMain(HINSTANCE,HINSTANCE,LPWSTR,int){int argc=0;char**argv=nullptr;QApplication a(argc,argv);Game g;g.show();return a.exec();}
#else
int main(int argc,char**argv){QApplication a(argc,argv);Game g;%5 return a.exec();}
#endif
)CPP")
        .arg(combinedScripts(), ents, componentCalls, esc(currentScene),launchCode,gamePixelHeight>gamePixelWidth?QString("true"):QString("false"),QString::number(gamePixelWidth),QString::number(gamePixelHeight),esc(gameName),esc(runtimeIcon));
  }
  bool writePortableProject(const QString&root,QString*error=nullptr){
    auto fail=[&](const QString&message){if(error)*error=message;return false;};
    QDir().mkpath(root+"/assets");
    QFile source(root+"/generated_game.cpp");if(!source.open(QIODevice::WriteOnly))return fail("Cannot write generated_game.cpp");source.write(runtime(true).toUtf8());source.close();
    QString qrcText="<RCC><qresource prefix=\"/assets\">\n";QSet<QString>aliases;
    QStringList portableAssets;if(!gameIconPath.isEmpty())portableAssets<<gameIconPath;for(auto*r:view->entities())portableAssets<<r->data(AssetRole).toString()<<r->data(AltAssetRole).toString();for(const QString&path:portableAssets){if(path.isEmpty()||!QFileInfo::exists(path))continue;QString alias=QFileInfo(path).fileName();if(aliases.contains(alias))continue;aliases.insert(alias);QString target=root+"/assets/"+alias;QFile::remove(target);if(!QFile::copy(path,target))return fail("Cannot copy asset: "+path);QString xml=alias;xml.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace("\"","&quot;");qrcText+=QString("<file alias=\"%1\">assets/%1</file>\n").arg(xml);}qrcText+="</qresource></RCC>\n";
    QFile qrc(root+"/assets.qrc");if(!qrc.open(QIODevice::WriteOnly))return fail("Cannot write assets.qrc");qrc.write(qrcText.toUtf8());qrc.close();
    QString exeBase=gameName;exeBase.replace(QRegularExpression("[^A-Za-z0-9_-]"),"_");if(exeBase.isEmpty())exeBase="GamePreview";if(!gameIconPath.isEmpty()){QImage icon(gameIconPath);if(!icon.isNull()&&icon.save(root+"/game.ico","ICO")){QFile rc(root+"/app.rc");if(rc.open(QIODevice::WriteOnly)){rc.write("IDI_ICON1 ICON DISCARDABLE \"game.ico\"\n");rc.close();}}}
    QFile cmake(root+"/CMakeLists.txt");if(!cmake.open(QIODevice::WriteOnly))return fail("Cannot write CMakeLists.txt");QString cmakeText=R"CMAKE(cmake_minimum_required(VERSION 3.21)
project(EnginePlusPlusGame LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
add_executable(GamePreview WIN32 MACOSX_BUNDLE generated_game.cpp assets.qrc)
set_property(TARGET GamePreview PROPERTY qt_no_entrypoint TRUE)
set_target_properties(GamePreview PROPERTIES OUTPUT_NAME "@EXE@")
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/app.rc")
  target_sources(GamePreview PRIVATE app.rc)
endif()
target_link_libraries(GamePreview PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)
if(MINGW)
  target_link_options(GamePreview PRIVATE -municode)
endif()
install(TARGETS GamePreview BUNDLE DESTINATION . RUNTIME DESTINATION bin)
)CMAKE";cmakeText.replace("@EXE@",exeBase);cmake.write(cmakeText.toUtf8());cmake.close();
    QFile win(root+"/build-windows.bat");if(!win.open(QIODevice::WriteOnly))return fail("Cannot write build-windows.bat");QString winText=R"BAT(@echo off
setlocal
set "PATH=C:\Qt\Tools\mingw1310_64\bin;C:\Qt\6.9.1\mingw_64\bin;%PATH%"
"C:\Qt\Tools\CMake_64\bin\cmake.exe" -S . -B build -G Ninja -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/Ninja/ninja.exe -DCMAKE_PREFIX_PATH=C:/Qt/6.9.1/mingw_64 -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 exit /b 1
"C:\Qt\Tools\CMake_64\bin\cmake.exe" --build build --config Release
if errorlevel 1 exit /b 1
"C:\Qt\6.9.1\mingw_64\bin\windeployqt.exe" --release --compiler-runtime --dir package build\@EXE@.exe
if errorlevel 1 exit /b 1
copy /Y build\@EXE@.exe package\@EXE@.exe
echo Windows package: %CD%\package
)BAT";winText.replace("@EXE@",exeBase);win.write(winText.toUtf8());win.close();
    QFile linux(root+"/build-linux.sh");if(!linux.open(QIODevice::WriteOnly))return fail("Cannot write build-linux.sh");linux.write(R"SH(#!/usr/bin/env bash
set -euo pipefail
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --install build --prefix package
echo "Linux build: $PWD/build/GamePreview"
if command -v linuxdeployqt >/dev/null 2>&1; then linuxdeployqt build/GamePreview -appimage; fi
)SH");linux.close();
    QFile mac(root+"/build-macos.sh");if(!mac.open(QIODevice::WriteOnly))return fail("Cannot write build-macos.sh");mac.write(R"SH(#!/usr/bin/env bash
set -euo pipefail
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
macdeployqt build/GamePreview.app -dmg
echo "macOS app: $PWD/build/GamePreview.app"
)SH");mac.close();
    QFile readme(root+"/BUILD_INSTRUCTIONS.txt");if(readme.open(QIODevice::WriteOnly)){readme.write("ENGINE++ CROSS-PLATFORM EXPORT\n\nWindows: run build-windows.bat.\nLinux: install Qt 6 Widgets + CMake, then run: chmod +x build-linux.sh && ./build-linux.sh\nmacOS: install Qt 6 + CMake, then run: chmod +x build-macos.sh && ./build-macos.sh\n\nAll images are embedded into the executable through assets.qrc.\nTouch input maps to scene.input.mouseX/mouseY/mouseDown on Android and touch screens.\n");readme.close();}
    return true;
  }
  void exportPortableProject(){tabs->setCurrentIndex(1);save();QString root=projectDir+"/desktop-export",error;if(!writePortableProject(root,&error)){QMessageBox::critical(this,"Desktop Export",error);return;}console->setText("CROSS-PLATFORM PROJECT CREATED\nWindows • Linux • macOS\n\n"+QDir::toNativeSeparators(root)+"\n\nUse the build script for the target operating system.");statusBar()->showMessage("Cross-platform CMake project created",6000);QMessageBox::information(this,"Desktop Export","Windows/Linux/macOS project created:\n"+QDir::toNativeSeparators(root));}
  void exportWindows(){
    if(desktopProcess&&desktopProcess->state()!=QProcess::NotRunning){QMessageBox::information(this,"Windows Export","Windows compilation is already running.");return;}
    tabs->setCurrentIndex(1);save();QString root=projectDir+"/windows-export",error;if(!writePortableProject(root,&error)){QMessageBox::critical(this,"Windows Export",error);return;}
    console->setText("WINDOWS EXE EXPORT (BACKGROUND)\nQt 6.9.1 • MinGW • CMake • Ninja\n\nCompiling and collecting DLL/plugins...");buildProgress->setRange(0,0);buildProgress->show();
    QString exeBase=gameName;exeBase.replace(QRegularExpression("[^A-Za-z0-9_-]"),"_");if(exeBase.isEmpty())exeBase="GamePreview";auto*p=new QProcess(this);desktopProcess=p;p->setWorkingDirectory(root);p->setProcessChannelMode(QProcess::MergedChannels);connect(p,&QProcess::readyReadStandardOutput,this,[this,p]{console->moveCursor(QTextCursor::End);console->insertPlainText(QString::fromLocal8Bit(p->readAllStandardOutput()));});connect(p,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this,p,root,exeBase](int code,QProcess::ExitStatus status){console->moveCursor(QTextCursor::End);console->insertPlainText(QString::fromLocal8Bit(p->readAll()));desktopProcess=nullptr;buildProgress->hide();p->deleteLater();QString exe=root+"/package/"+exeBase+".exe";if(status!=QProcess::NormalExit||code!=0||!QFileInfo::exists(exe)){console->append("\nWINDOWS EXPORT FAILED");QMessageBox::critical(this,"Windows Export","Compilation failed. See the build console.");return;}console->append("\nWINDOWS EXPORT SUCCESSFUL\n"+QDir::toNativeSeparators(exe));statusBar()->showMessage("Windows EXE package created",7000);QMessageBox::information(this,"Windows Export","Ready Windows game:\n"+QDir::toNativeSeparators(exe));});p->start("cmd.exe",{"/d","/c","build-windows.bat"});if(!p->waitForStarted(5000)){desktopProcess=nullptr;buildProgress->hide();p->deleteLater();QMessageBox::critical(this,"Windows Export","Cannot start build-windows.bat");}
  }
  void androidFail(const QString &message){buildProgress->hide();statusBar()->showMessage("Android build failed",6000);console->append("\n\nANDROID BUILD FAILED: "+message);QMessageBox::critical(this,"Android APK",message+"\n\nSee the build console for details.");}
  void startAndroidCommand(const QString &title,const QString &program,const QStringList &args){
    console->append("\n\n== "+title+" ==\n> "+program+" "+args.join(' '));statusBar()->showMessage(title+" • editor remains responsive");
    auto *p=new QProcess(this);androidProcess=p;p->setWorkingDirectory(androidRoot);p->setProcessChannelMode(QProcess::MergedChannels);
    QProcessEnvironment env=QProcessEnvironment::systemEnvironment();env.insert("ANDROID_SDK_ROOT",androidSdk);env.insert("ANDROID_HOME",androidSdk);env.insert("ANDROID_NDK_ROOT",androidNdk);QString javaHome="C:/Program Files/Android/Android Studio/jbr";if(QFileInfo::exists(javaHome))env.insert("JAVA_HOME",javaHome);p->setProcessEnvironment(env);
    connect(p,&QProcess::readyReadStandardOutput,this,[this,p]{console->moveCursor(QTextCursor::End);console->insertPlainText(QString::fromUtf8(p->readAllStandardOutput()));});
    connect(p,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this,p,title](int code,QProcess::ExitStatus state){console->moveCursor(QTextCursor::End);console->insertPlainText(QString::fromUtf8(p->readAll()));if(androidProcess==p)androidProcess=nullptr;p->deleteLater();if(state!=QProcess::NormalExit||code!=0){androidFail(title+QString(" exited with code %1").arg(code));return;}androidNext();});
    p->start(program,args);if(!p->waitForStarted(5000)){androidProcess=nullptr;p->deleteLater();androidFail("Cannot start: "+program);}
  }
  void androidNext(){
    if(androidStage==0){androidStage=1;startAndroidCommand("Building native ARM64 C++ library",androidCmake,{"--build",androidBuild,"--config","Release"});return;}
    if(androidStage==1){QDirIterator settings(androidBuild,{"android-*-deployment-settings.json"},QDir::Files,QDirIterator::Subdirectories);if(!settings.hasNext()){androidFail("Qt deployment settings were not generated. The .so is only an intermediate file; APK packaging cannot start.");return;}androidDeployJson=settings.next();androidStage=2;QDir().mkpath(androidApk);QString key=QDir::homePath()+"/.android/debug.keystore";if(!QFileInfo::exists(key)){androidFail("Android debug signing key is missing: "+key);return;}QFile::remove(androidFinalApk);startAndroidCommand("Packaging and signing installable APK",androidDeploy,{"--input",androidDeployJson,"--output",androidApk,"--android-platform","android-35","--gradle","--release","--sign",key,"androiddebugkey","--storepass","android","--keypass","android","--apk",androidFinalApk,"--verbose"});return;}
    if(!QFileInfo::exists(androidFinalApk)||QFileInfo(androidFinalApk).size()<1024*1024){androidFail("Signed APK was not created at "+androidFinalApk);return;}buildProgress->hide();statusBar()->showMessage("Signed Android APK created",8000);console->append("\n\nANDROID BUILD SUCCESSFUL\nSIGNED INSTALLABLE APK: "+QDir::toNativeSeparators(androidFinalApk));QMessageBox::information(this,"Android APK","Signed APK created successfully:\n"+QDir::toNativeSeparators(androidFinalApk));
  }
  void exportAndroid(){
    if(androidProcess&&androidProcess->state()!=QProcess::NotRunning){QMessageBox::information(this,"Android Export","Android compilation is already running. Progress is visible in the console.");return;}
    tabs->setCurrentIndex(1);save();androidRoot=projectDir+"/android-export";androidBuild=androidRoot+"/build";androidApk=androidBuild+"/android-build";androidFinalApk=androidRoot+"/EnginePlusPlus-installable-arm64.apk";androidSdk=QDir::homePath()+"/AppData/Local/Android/Sdk";androidNdk=androidSdk+"/ndk/27.2.12479018";androidCmake="C:/Qt/Tools/CMake_64/bin/cmake.exe";androidNinja="C:/Qt/Tools/Ninja/ninja.exe";androidDeploy="C:/Qt/6.9.1/mingw_64/bin/androiddeployqt.exe";
    QString qtToolchain="C:/Qt/6.9.1/android_arm64_v8a/lib/cmake/Qt6/qt.toolchain.cmake";for(const QString &required:{androidNdk,androidCmake,androidNinja,androidDeploy,qtToolchain})if(!QFileInfo::exists(required)){androidFail("Required Android/Qt tool is missing: "+required);return;}
    QDir().mkpath(androidRoot);QFile source(androidRoot+"/generated_game.cpp");if(!source.open(QIODevice::WriteOnly)){androidFail("Cannot write generated_game.cpp");return;}source.write(runtime(true).toUtf8());source.close();
    QString packageAssets=androidRoot+"/android-package/assets/engine";QDir().mkpath(packageAssets);QString qrcText="<RCC><qresource prefix=\"/assets\">\n";QSet<QString> aliases;for(auto*r:view->entities())for(const QString &path:{r->data(AssetRole).toString(),r->data(AltAssetRole).toString()}){if(path.isEmpty()||!QFileInfo::exists(path))continue;QString alias=QFileInfo(path).fileName();if(aliases.contains(alias))continue;aliases.insert(alias);QString packaged=packageAssets+"/"+alias;QFile::remove(packaged);if(!QFile::copy(path,packaged)){androidFail("Cannot package image: "+path);return;}QString xml=QDir::fromNativeSeparators(QFileInfo(path).absoluteFilePath());xml.replace("&","&amp;").replace("<","&lt;").replace(">","&gt;").replace("\"","&quot;");qrcText+=QString("<file alias=\"%1\">%2</file>\n").arg(alias,xml);}qrcText+="</qresource></RCC>\n";QFile qrc(androidRoot+"/assets.qrc");if(!qrc.open(QIODevice::WriteOnly)){androidFail("Cannot write assets.qrc");return;}qrc.write(qrcText.toUtf8());qrc.close();
    QString androidIcon;if(!gameIconPath.isEmpty()&&QFileInfo::exists(gameIconPath)){QImage icon(gameIconPath);androidIcon=androidRoot+"/game_icon.png";if(!icon.isNull())icon.scaled(512,512,Qt::KeepAspectRatio,Qt::SmoothTransformation).save(androidIcon,"PNG");}
    QFile cmake(androidRoot+"/CMakeLists.txt");if(!cmake.open(QIODevice::WriteOnly)){androidFail("Cannot write Android CMakeLists.txt");return;}QString androidCmakeText=QString(R"CMAKE(cmake_minimum_required(VERSION 3.21)
project(EnginePlusPlusGame LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
qt_add_executable(GamePreview MANUAL_FINALIZATION generated_game.cpp assets.qrc)
target_link_libraries(GamePreview PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)
set_target_properties(GamePreview PROPERTIES QT_ANDROID_PACKAGE_NAME "com.engineplusplus.game" QT_ANDROID_APP_NAME "%1" QT_ANDROID_APP_ICON "%2" QT_ANDROID_PERMISSIONS "android.permission.VIBRATE" QT_ANDROID_MIN_SDK_VERSION 28 QT_ANDROID_TARGET_SDK_VERSION 35 QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android-package")
qt_finalize_executable(GamePreview)
)CMAKE").arg(gameName,androidIcon);cmake.write(androidCmakeText.toUtf8());cmake.close();
    console->setText("ANDROID APK EXPORT (BACKGROUND)\nQt 6.9.1 • ARM64 • API 35 • NDK r27c\nThe editor remains responsive. The .so is intermediate; Gradle packages the final APK.");buildProgress->setRange(0,0);buildProgress->show();androidStage=0;
    QStringList configure={"-S",androidRoot,"-B",androidBuild,"-G","Ninja","-DCMAKE_MAKE_PROGRAM="+androidNinja,"-DCMAKE_TOOLCHAIN_FILE="+qtToolchain,"-DQT_CHAINLOAD_TOOLCHAIN_FILE="+androidNdk+"/build/cmake/android.toolchain.cmake","-DQT_HOST_PATH=C:/Qt/6.9.1/mingw_64","-DANDROID_SDK_ROOT="+androidSdk,"-DANDROID_NDK_ROOT="+androidNdk,"-DCMAKE_ANDROID_NDK="+androidNdk,"-DANDROID_ABI=arm64-v8a","-DANDROID_PLATFORM=android-35","-DCMAKE_BUILD_TYPE=Release"};
    startAndroidCommand("Configuring Android project",androidCmake,configure);
  }
  bool process(const QString &program,const QStringList &args,const QString &working,int timeout=600000){
    QProcess p;p.setWorkingDirectory(working);p.setProcessChannelMode(QProcess::MergedChannels);p.start(program,args);
    if(!p.waitForFinished(timeout)){p.kill();console->append("\nTIMEOUT: "+program);return false;}
    QString output=QString::fromUtf8(p.readAll());if(!output.trimmed().isEmpty())console->append("\n"+output);
    return p.exitStatus()==QProcess::NormalExit&&p.exitCode()==0;
  }
  void exportAndroidBlockingLegacy(){
    tabs->setCurrentIndex(1);save();QString root=dir()+"/android-export",build=root+"/build",apk=root+"/apk";
    QDir().mkpath(root);QFile source(root+"/generated_game.cpp");source.open(QIODevice::WriteOnly);source.write(runtime().toUtf8());source.close();
    QFile cmake(root+"/CMakeLists.txt");cmake.open(QIODevice::WriteOnly);cmake.write(R"CMAKE(cmake_minimum_required(VERSION 3.21)
project(EnginePlusPlusGame LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
find_package(Qt6 REQUIRED COMPONENTS Widgets Multimedia MultimediaWidgets)
qt_add_executable(GamePreview generated_game.cpp)
target_link_libraries(GamePreview PRIVATE Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets)
set_target_properties(GamePreview PROPERTIES QT_ANDROID_PACKAGE_NAME "com.engineplusplus.game" QT_ANDROID_APP_NAME "Engine Plus Plus Game")
)CMAKE");cmake.close();
    QString sdk="C:/Users/user/AppData/Local/Android/Sdk",ndk=sdk+"/ndk/27.2.12479018";
    if(!QFileInfo::exists(ndk)){QMessageBox::critical(this,"Android Export","NDK r27c is missing: "+ndk);return;}
    console->setText("ANDROID ARM64 EXPORT\nQt 6.9.1 • API 35 • NDK r27c\nConfiguring CMake...");QApplication::processEvents();
    QString cmakeExe="C:/Qt/Tools/CMake_64/bin/cmake.exe",ninja="C:/Qt/Tools/Ninja/ninja.exe";
    QStringList configure={"-S",root,"-B",build,"-G","Ninja","-DCMAKE_MAKE_PROGRAM="+ninja,"-DCMAKE_TOOLCHAIN_FILE=C:/Qt/6.9.1/android_arm64_v8a/lib/cmake/Qt6/qt.toolchain.cmake","-DQT_CHAINLOAD_TOOLCHAIN_FILE="+ndk+"/build/cmake/android.toolchain.cmake","-DQT_HOST_PATH=C:/Qt/6.9.1/mingw_64","-DANDROID_SDK_ROOT="+sdk,"-DANDROID_NDK_ROOT="+ndk,"-DCMAKE_ANDROID_NDK="+ndk,"-DANDROID_ABI=arm64-v8a","-DANDROID_PLATFORM=android-35","-DCMAKE_BUILD_TYPE=Release"};
    if(!process(cmakeExe,configure,root)){console->setText("ANDROID CONFIGURE FAILED\n"+console->toPlainText());return;}console->append("\nBuilding native ARM64 library...");
    if(!process(cmakeExe,{"--build",build,"--config","Release"},root)){console->setText("ANDROID C++ BUILD FAILED\n"+console->toPlainText());return;}
    QDir bd(build);QStringList json=bd.entryList({"android-*-deployment-settings.json"},QDir::Files);if(json.isEmpty()){console->setText("ANDROID DEPLOY SETTINGS NOT FOUND\n"+console->toPlainText());return;}
    console->append("\nPackaging APK with Gradle...");QString deploy="C:/Qt/6.9.1/mingw_64/bin/androiddeployqt.exe";
    if(!process(deploy,{"--input",build+"/"+json.first(),"--output",apk,"--android-platform","android-35","--gradle"},root)){console->setText("APK PACKAGING FAILED\n"+console->toPlainText());return;}
    QDirIterator it(apk,{"*.apk"},QDir::Files,QDirIterator::Subdirectories);QString result;if(it.hasNext())result=it.next();
    if(result.isEmpty()){console->setText("APK FILE NOT FOUND\n"+console->toPlainText());return;}console->setText("ANDROID BUILD SUCCESSFUL\nAPK: "+result+"\n"+console->toPlainText());QMessageBox::information(this,"Android APK","APK created:\n"+result);
  }
  void run() {
    tabs->setCurrentIndex(1);
    save();
    QFile f(dir() + "/generated_game.cpp");
    if (!f.open(QIODevice::WriteOnly)) {
      console->setText("Cannot write generated_game.cpp");
      return;
    }
    f.write(runtime().toUtf8());
    f.close();
    console->setText("⚡ Optimizing C++: -O3, LTO, NDEBUG\nCompiling scene + "
                     "script + graphics runtime...\n");
    QApplication::processEvents();
    QString inc = "C:/msys64/ucrt64/include/qt6", lib = "C:/msys64/ucrt64/lib",
            exe = dir() + "/GamePreview.exe";
    QStringList a = {"-std=c++20",
                     "-O3",
                     "-flto",
                     "-DNDEBUG",
                     "-Wno-sfinae-incomplete",
                     "-mwindows",
                     "-municode",
                     dir() + "/generated_game.cpp",
                     "-I" + inc,
                     "-I" + inc + "/QtCore",
                     "-I" + inc + "/QtGui",
                     "-I" + inc + "/QtWidgets",
                     "-I" + inc + "/QtMultimedia",
                     "-I" + inc + "/QtMultimediaWidgets",
                     "-L" + lib,
                     "-lQt6Widgets",
                     "-lQt6MultimediaWidgets",
                     "-lQt6Multimedia",
                     "-lQt6Gui",
                     "-lQt6Core",
                     "-o",
                     exe};
    if(buildProcess&&buildProcess->state()!=QProcess::NotRunning){console->append("\nBuild is already running.");return;}
    buildProcess=new QProcess(this);buildProcess->setWorkingDirectory(dir());buildProcess->setProcessChannelMode(QProcess::MergedChannels);
    buildProgress->setRange(0,0);buildProgress->show();statusBar()->showMessage("Compiling in background • editor remains responsive");
    connect(buildProcess,&QProcess::readyReadStandardOutput,this,[this]{console->moveCursor(QTextCursor::End);console->insertPlainText(QString::fromUtf8(buildProcess->readAllStandardOutput()));});
    connect(buildProcess,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this,exe](int exitCode,QProcess::ExitStatus status){buildProgress->hide();if(status!=QProcess::NormalExit||exitCode!=0){console->append("\nBUILD FAILED");statusBar()->showMessage("Build failed",5000);return;}console->append("\nBUILD SUCCESSFUL • starting GamePreview.exe");statusBar()->showMessage("Build successful",4000);QProcess::startDetached(exe,{},dir());});
    buildProcess->start(compiler,a);
  }
public:
  QByteArray generatedRuntimeForTest(){return runtime().toUtf8();}
  bool exportPortableForTest(const QString&path){return writePortableProject(path);}
};
int main(int argc, char **argv) {
  QApplication a(argc, argv);
  a.setStyle("Fusion");
  Window w;
  if(argc==3&&QString::fromLocal8Bit(argv[1])=="--dump-runtime"){QFile output(QString::fromLocal8Bit(argv[2]));if(!output.open(QIODevice::WriteOnly))return 2;output.write(w.generatedRuntimeForTest());return 0;}
  if(argc==3&&QString::fromLocal8Bit(argv[1])=="--export-portable")return w.exportPortableForTest(QString::fromLocal8Bit(argv[2]))?0:3;
  w.show();
  return a.exec();
}
