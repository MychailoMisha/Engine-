// Lines98_Real_Balls_Full.cpp
// Complete standalone Color Lines game for Engine++ v19.
// Replace the whole Main.cpp script with this file. Do not keep other onStart/onUpdate.

namespace RealLines98 {

constexpr int Columns = 9;
constexpr int Rows = 9;
constexpr int Cell = 40;
constexpr int Empty = -1;
constexpr int ColorCount = 7;
constexpr int LineLength = 5;
constexpr int SpawnCount = 3;
constexpr float OriginX = -Columns * Cell / 2.0f;
constexpr float OriginY = -160.0f;

struct GameState {
    QVector<int> board;
    QVector<int> nextColors;
    QString sceneName = "Scene 1";
    int cursor = 0;
    int hover = -1;
    int selected = -1;
    int score = 0;
    QString message;
    bool gameOver = false;
    bool dirty = true;
    bool oldLeft = false;
    bool oldRight = false;
    bool oldUp = false;
    bool oldDown = false;
    bool oldSpace = false;
    bool oldMouseDown = false;
};

GameState game;

const QVector<QColor> BallColors = {
    QColor("#ff365f"), QColor("#2f9bff"), QColor("#38d878"),
    QColor("#ffd238"), QColor("#a970ff"), QColor("#ff8a30"),
    QColor("#25d8db")
};

int indexOf(int x, int y) { return y * Columns + x; }
int xOf(int cell) { return cell % Columns; }
int yOf(int cell) { return cell / Columns; }
bool inside(int x, int y) { return x >= 0 && y >= 0 && x < Columns && y < Rows; }

QPointF cellCenter(int cell) {
    return QPointF(OriginX + (xOf(cell) + 0.5f) * Cell,
                   OriginY + (yOf(cell) + 0.5f) * Cell);
}

Entity makeEntity(const QString& type, const QString& tag) {
    Entity entity{};
    entity.type = type;
    entity.behavior = tag;
    entity.sceneName = game.sceneName;
    entity.visible = true;
    entity.hitbox = false;
    entity.opacity = 1.0f;
    entity.scale = 1.0f;
    entity.color = QColor("#ffffff");
    entity.fill = entity.color;
    entity.stroke = QColor("#ffffff");
    entity.width = 40;
    entity.height = 40;
    entity.cornerRadius = 8;
    return entity;
}

void removeGenerated(Scene& scene) {
    for (int i = scene.e.size() - 1; i >= 0; --i)
        if (scene.e[i].behavior.startsWith("L98_")) scene.e.removeAt(i);
}

void addText(Scene& scene, const QString& name, const QString& text,
             float x, float y, float width, int sizeRole = 0) {
    Entity label = makeEntity("Text", "L98_TEXT");
    label.name = name;
    label.text = text;
    label.x = x;
    label.y = y;
    label.width = width;
    label.height = 34;
    label.z = 30;
    label.color = sizeRole == 1 ? QColor("#ffffff") : QColor("#aebbd8");
    scene.e.push_back(label);
}

void addCell(Scene& scene, int cell) {
    Entity tile = makeEntity("Panel", "L98_CELL");
    QPointF center = cellCenter(cell);
    tile.x = center.x();
    tile.y = center.y();
    tile.width = Cell - 3;
    tile.height = Cell - 3;
    tile.cornerRadius = 7;
    tile.z = 1;
    tile.color = ((xOf(cell) + yOf(cell)) & 1)
        ? QColor("#18233a") : QColor("#1d2942");
    if (cell == game.cursor && game.hover < 0) tile.color = QColor("#344563");
    if (cell == game.hover && game.board[cell] != Empty) tile.color = QColor("#3c5275");
    if (cell == game.hover && game.selected >= 0 && game.board[cell] == Empty)
        tile.color = QColor("#274b4a");
    if (cell == game.selected) tile.color = QColor("#7259df");
    scene.e.push_back(tile);
}

void addBall(Scene& scene, int cell, int color, bool preview = false) {
    // ASFO Territory with Circle shape is rendered as a true antialiased ellipse.
    QPointF center = preview
        ? QPointF(270.0f + cell * 31.0f, -205.0f)
        : cellCenter(cell);
    if (!preview) {
        Entity shadow = makeEntity("ASFO Territory", "L98_SHADOW");
        shadow.x = center.x() + 2.5f;
        shadow.y = center.y() + 3.5f;
        shadow.width = 31;
        shadow.height = 31;
        shadow.shape = "Circle";
        shadow.color = QColor("#000000");
        shadow.fill = QColor("#000000");
        shadow.opacity = 0.42f;
        shadow.z = 12;
        scene.e.push_back(shadow);
    }
    Entity ball = makeEntity("ASFO Territory", preview ? "L98_PREVIEW" : "L98_BALL");
    ball.x = center.x();
    ball.y = center.y();
    ball.width = preview ? 22 : 29;
    ball.height = ball.width;
    ball.shape = "Circle";
    ball.color = BallColors[color].lighter(135);
    ball.fill = BallColors[color];
    ball.stroke = BallColors[color].lighter(170);
    ball.z = preview ? 35 : 15;
    ball.uiValue = cell;
    if (!preview && cell == game.selected) {
        ball.scale = 1.18f;
        ball.behavior = "L98_SELECTED";
    }
    scene.e.push_back(ball);
    if (!preview) {
        Entity shine = makeEntity("ASFO Territory", "L98_SHINE");
        shine.x = center.x() - 5.5f;
        shine.y = center.y() - 6.0f;
        shine.width = 7;
        shine.height = 7;
        shine.shape = "Circle";
        shine.color = QColor("#ffffff");
        shine.fill = QColor("#ffffff");
        shine.opacity = 0.58f;
        shine.z = 17;
        scene.e.push_back(shine);
    }
}

QString statusText() {
    if (game.gameOver) return "GAME OVER — click the board or press SPACE";
    if (!game.message.isEmpty()) return game.message;
    if (game.selected >= 0) return "Choose an empty cell with a free path";
    return "Click a ball, then click an empty cell";
}

void rebuildScene(Scene& scene) {
    removeGenerated(scene);

    Entity background = makeEntity("Panel", "L98_BACKGROUND");
    background.x = 0; background.y = 0; background.width = 790; background.height = 440;
    background.cornerRadius = 24; background.color = QColor("#080d18"); background.z = -20;
    scene.e.push_back(background);
    Entity header = makeEntity("Glass Panel", "L98_HEADER");
    header.x = 0; header.y = -205; header.width = 760; header.height = 46;
    header.cornerRadius = 16; header.color = QColor("#222c48"); header.glass = true; header.z = -5;
    scene.e.push_back(header);
    Entity footer = makeEntity("Panel", "L98_FOOTER");
    footer.x = 0; footer.y = 207; footer.width = 560; footer.height = 32;
    footer.cornerRadius = 14; footer.color = QColor("#111a2c"); footer.z = -4;
    scene.e.push_back(footer);

    Entity boardPanel = makeEntity("Panel", "L98_BOARD");
    boardPanel.x = 0;
    boardPanel.y = OriginY + Rows * Cell / 2.0f;
    boardPanel.width = Columns * Cell + 18;
    boardPanel.height = Rows * Cell + 18;
    boardPanel.cornerRadius = 18;
    boardPanel.color = QColor("#0e1628");
    boardPanel.z = 0;
    scene.e.push_back(boardPanel);

    for (int cell = 0; cell < Columns * Rows; ++cell) addCell(scene, cell);
    for (int cell = 0; cell < game.board.size(); ++cell)
        if (game.board[cell] != Empty) addBall(scene, cell, game.board[cell]);

    addText(scene, "Lines98 Title", "LINES 98", -290, -205, 170, 1);
    addText(scene, "Lines98 Score", QString("SCORE  %1").arg(game.score), -80, -205, 190, 1);
    addText(scene, "Lines98 Next", "NEXT", 190, -205, 80);
    for (int i = 0; i < game.nextColors.size(); ++i)
        addBall(scene, i, game.nextColors[i], true);
    addText(scene, "Lines98 Status", statusText(), 0, 207, 520);
    if (game.gameOver) {
        Entity overlay = makeEntity("Glass Panel", "L98_GAMEOVER");
        overlay.x = 0; overlay.y = 20; overlay.width = 310; overlay.height = 115;
        overlay.cornerRadius = 22; overlay.color = QColor("#311d43"); overlay.glass = true; overlay.z = 25;
        scene.e.push_back(overlay);
        addText(scene, "Lines98 Game Over", "GAME OVER", 0, 2, 270, 1);
        addText(scene, "Lines98 Restart", "Tap to restart", 0, 42, 250);
    }
    game.dirty = false;
}

QVector<int> freeCells() {
    QVector<int> result;
    for (int i = 0; i < game.board.size(); ++i)
        if (game.board[i] == Empty) result.push_back(i);
    return result;
}

void rollNextColors() {
    game.nextColors.clear();
    for (int i = 0; i < SpawnCount; ++i)
        game.nextColors.push_back(QRandomGenerator::global()->bounded(ColorCount));
}

QVector<int> placeRandomBalls(int count, bool usePreview) {
    QVector<int> placed;
    for (int i = 0; i < count; ++i) {
        QVector<int> free = freeCells();
        if (free.isEmpty()) { game.gameOver = true; break; }
        int cell = free[QRandomGenerator::global()->bounded(free.size())];
        int color = usePreview && i < game.nextColors.size()
            ? game.nextColors[i]
            : QRandomGenerator::global()->bounded(ColorCount);
        game.board[cell] = color;
        placed.push_back(cell);
    }
    if (usePreview) rollNextColors();
    game.dirty = true;
    return placed;
}

bool hasFreePath(int from, int destination) {
    if (from < 0 || destination < 0 || destination >= game.board.size() ||
        from == destination || game.board[destination] != Empty) return false;

    QVector<bool> visited(game.board.size(), false);
    QQueue<int> queue;
    queue.enqueue(from);
    visited[from] = true;
    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    while (!queue.isEmpty()) {
        int cell = queue.dequeue();
        for (int direction = 0; direction < 4; ++direction) {
            int x = xOf(cell) + dx[direction];
            int y = yOf(cell) + dy[direction];
            if (!inside(x, y)) continue;
            int next = indexOf(x, y);
            if (visited[next] || game.board[next] != Empty) continue;
            if (next == destination) return true;
            visited[next] = true;
            queue.enqueue(next);
        }
    }
    return false;
}

int clearLinesThrough(const QVector<int>& seeds) {
    QSet<int> remove;
    const int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

    // Only a continuous line through a ball that has just moved/spawned counts.
    // A single empty cell stops both scans, so patterns with a gap never clear.
    for (int seed : seeds) {
        if (seed < 0 || seed >= game.board.size()) continue;
        int color = game.board[seed];
        if (color == Empty) continue;
        for (const auto& direction : directions) {
            QVector<int> line{seed};
            for (int sign : {-1, 1}) {
                int x = xOf(seed) + direction[0] * sign;
                int y = yOf(seed) + direction[1] * sign;
                while (inside(x, y) && game.board[indexOf(x, y)] == color) {
                    line.push_back(indexOf(x, y));
                    x += direction[0] * sign;
                    y += direction[1] * sign;
                }
            }
            if (line.size() >= LineLength)
                for (int cell : line) remove.insert(cell);
        }
    }

    if (remove.isEmpty()) return 0;
    for (int cell : remove) game.board[cell] = Empty;
    int removed = remove.size();
    game.score += removed * 10 + qMax(0, removed - LineLength) * 5;
    game.dirty = true;
    return removed;
}

void reset(Scene& scene) {
    // Use the current scene of the old Grid Map, but hide that editor object.
    for (auto& entity : scene.e) {
        if (entity.type == "Grid Map") {
            game.sceneName = entity.sceneName;
            entity.visible = false;
        }
        if (entity.type == "Character") entity.visible = false;
    }

    game.board.fill(Empty, Columns * Rows);
    game.cursor = 0;
    game.hover = -1;
    game.selected = -1;
    game.score = 0;
    game.message = "Select any colored ball";
    game.gameOver = false;
    rollNextColors();
    placeRandomBalls(5, false);
    game.dirty = true;
    rebuildScene(scene);
}

int cellAt(float worldX, float worldY) {
    int x = qFloor((worldX - OriginX) / Cell);
    int y = qFloor((worldY - OriginY) / Cell);
    return inside(x, y) ? indexOf(x, y) : -1;
}

void moveCursor(int dx, int dy) {
    int x = qBound(0, xOf(game.cursor) + dx, Columns - 1);
    int y = qBound(0, yOf(game.cursor) + dy, Rows - 1);
    int next = indexOf(x, y);
    if (next != game.cursor) { game.cursor = next; game.dirty = true; }
}

void activate(Scene& scene, int cell) {
    if (cell < 0) return;
    game.cursor = cell;

    if (game.gameOver) { reset(scene); return; }

    if (game.board[cell] != Empty) {
        game.selected = cell;
        game.message = "Ball selected — now choose an empty cell";
        game.dirty = true;
        return;
    }

    if (game.selected < 0) {
        game.message = "First click a ball, not an empty cell";
        game.dirty = true;
        return;
    }

    if (!hasFreePath(game.selected, cell)) {
        game.message = "No free path to that cell";
        game.dirty = true;
        return;
    }

    game.board[cell] = game.board[game.selected];
    game.board[game.selected] = Empty;
    game.selected = -1;

    int removed = clearLinesThrough({cell});
    if (removed == 0) {
        QVector<int> spawned = placeRandomBalls(SpawnCount, true);
        removed = clearLinesThrough(spawned);
    }
    game.message = removed > 0
        ? QString("Great! Removed %1 balls").arg(removed)
        : "Three new balls appeared";
    if (freeCells().isEmpty()) game.gameOver = true;
    game.dirty = true;
}

void animateSelected(Scene& scene) {
    for (auto& entity : scene.e) {
        if (entity.behavior == "L98_SELECTED") {
            entity.scale = 1.12f + 0.07f * std::sin(scene.time * 6.0f);
            entity.y = cellCenter(game.selected).y() -
                       qMax(0.0f, std::sin(scene.time * 6.0f)) * 4.0f;
        }
    }
}

} // namespace RealLines98

void onStart(Scene& scene) {
    RealLines98::reset(scene);
}

void onUpdate(Scene& scene, float /*dt*/) {
    using namespace RealLines98;

    const bool left = scene.input.left;
    const bool right = scene.input.right;
    const bool up = scene.input.up;
    const bool down = scene.input.down;
    const bool space = scene.input.space;
    const bool mouseDown = scene.input.mouseDown;

    int hover = cellAt(scene.input.mouseX, scene.input.mouseY);
    if (hover != game.hover) {
        game.hover = hover;
        game.dirty = true;
    }

    if (left && !game.oldLeft) moveCursor(-1, 0);
    if (right && !game.oldRight) moveCursor(1, 0);
    if (up && !game.oldUp) moveCursor(0, -1);
    if (down && !game.oldDown) moveCursor(0, 1);
    if (space && !game.oldSpace) activate(scene, game.cursor);
    if (mouseDown && !game.oldMouseDown && hover >= 0) activate(scene, hover);

    game.oldLeft = left;
    game.oldRight = right;
    game.oldUp = up;
    game.oldDown = down;
    game.oldSpace = space;
    game.oldMouseDown = mouseDown;

    if (game.dirty) rebuildScene(scene);
    animateSelected(scene);
}
