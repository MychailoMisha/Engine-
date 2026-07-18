// Logic_Game.cpp — Lines 98 logic for an Engine++ Grid Map.
// Main.cpp owns onStart/onUpdate. Call lines98Start/lines98Update there.

namespace Lines98 {

constexpr int Empty = -1;
constexpr int Colors = 7;
constexpr int SpawnPerTurn = 3;
constexpr int LineLength = 5;

struct State {
    int columns = 0;
    int rows = 0;
    int cursor = 0;
    int selected = -1;
    int score = 0;
    bool started = false;
    bool gameOver = false;
    bool oldLeft = false;
    bool oldRight = false;
    bool oldUp = false;
    bool oldDown = false;
    bool oldSpace = false;
    QVector<int> board;
};

State game;

const QVector<QColor> palette = {
    QColor("#ff4d6d"), QColor("#4dabf7"), QColor("#51cf66"),
    QColor("#ffd43b"), QColor("#b197fc"), QColor("#ff922b"),
    QColor("#22b8cf")
};

int index(int x, int y) { return y * game.columns + x; }
int cellX(int cell) { return cell % game.columns; }
int cellY(int cell) { return cell / game.columns; }
bool inside(int x, int y) {
    return x >= 0 && y >= 0 && x < game.columns && y < game.rows;
}

QString keyFor(int cell) {
    return QString::number(cellX(cell)) + "," + QString::number(cellY(cell));
}

Entity* grid(Scene& scene) {
    if (auto* named = scene.find("Grid Layout")) return named;
    return scene.findType("Grid Map");
}

void updateLabels(Scene& scene, const QString& message = {}) {
    if (auto* score = scene.find("Score"))
        score->text = QString("SCORE: %1").arg(game.score);
    if (auto* status = scene.find("Status")) {
        if (game.gameOver) status->text = "GAME OVER — press R/Space";
        else if (!message.isEmpty()) status->text = message;
        else if (game.selected >= 0) status->text = "Choose an empty destination";
        else status->text = "Select a ball";
    }
}

void draw(Scene& scene) {
    Entity* map = grid(scene);
    if (!map) return;

    map->cellColors.clear();
    for (int cell = 0; cell < game.board.size(); ++cell) {
        const int color = game.board[cell];
        if (color != Empty) map->cellColors[keyFor(cell)] = palette[color];
    }

    // The cursor and selected ball are made brighter, so no extra object is needed.
    if (game.cursor >= 0 && game.cursor < game.board.size()) {
        QColor cursorColor = game.board[game.cursor] == Empty
            ? QColor("#35405f") : palette[game.board[game.cursor]].lighter(155);
        map->cellColors[keyFor(game.cursor)] = cursorColor;
    }
    if (game.selected >= 0 && game.selected < game.board.size())
        map->cellColors[keyFor(game.selected)] = palette[game.board[game.selected]].lighter(190);
}

QVector<int> emptyCells() {
    QVector<int> result;
    for (int i = 0; i < game.board.size(); ++i)
        if (game.board[i] == Empty) result.push_back(i);
    return result;
}

void spawnBalls(int amount) {
    for (int count = 0; count < amount; ++count) {
        QVector<int> free = emptyCells();
        if (free.isEmpty()) { game.gameOver = true; return; }
        int cell = free[QRandomGenerator::global()->bounded(free.size())];
        game.board[cell] = QRandomGenerator::global()->bounded(Colors);
    }
    game.gameOver = emptyCells().isEmpty();
}

bool hasPath(int from, int to) {
    if (from == to || to < 0 || to >= game.board.size() || game.board[to] != Empty)
        return false;

    QVector<bool> visited(game.board.size(), false);
    QQueue<int> queue;
    queue.enqueue(from);
    visited[from] = true;
    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    while (!queue.isEmpty()) {
        int cell = queue.dequeue();
        for (int direction = 0; direction < 4; ++direction) {
            int x = cellX(cell) + dx[direction];
            int y = cellY(cell) + dy[direction];
            if (!inside(x, y)) continue;
            int next = index(x, y);
            if (visited[next] || game.board[next] != Empty) continue;
            if (next == to) return true;
            visited[next] = true;
            queue.enqueue(next);
        }
    }
    return false;
}

int removeCompletedLines() {
    QSet<int> remove;
    const int directions[4][2] = {{1,0}, {0,1}, {1,1}, {1,-1}};

    for (int y = 0; y < game.rows; ++y) {
        for (int x = 0; x < game.columns; ++x) {
            int start = index(x, y);
            int color = game.board[start];
            if (color == Empty) continue;

            for (const auto& direction : directions) {
                int previousX = x - direction[0];
                int previousY = y - direction[1];
                if (inside(previousX, previousY) &&
                    game.board[index(previousX, previousY)] == color) continue;

                QVector<int> line;
                int scanX = x;
                int scanY = y;
                while (inside(scanX, scanY) &&
                       game.board[index(scanX, scanY)] == color) {
                    line.push_back(index(scanX, scanY));
                    scanX += direction[0];
                    scanY += direction[1];
                }
                if (line.size() >= LineLength)
                    for (int cell : line) remove.insert(cell);
            }
        }
    }

    for (int cell : remove) game.board[cell] = Empty;
    game.score += remove.size() * 10;
    return remove.size();
}

void reset(Scene& scene) {
    Entity* map = grid(scene);
    if (!map) {
        game.started = false;
        updateLabels(scene, "Add a Grid Map named Grid Layout");
        return;
    }

    game.columns = qMax(5, map->columns);
    game.rows = qMax(5, map->rows);
    map->columns = game.columns;
    map->rows = game.rows;
    game.board.fill(Empty, game.columns * game.rows);
    game.cursor = 0;
    game.selected = -1;
    game.score = 0;
    game.gameOver = false;
    game.started = true;
    spawnBalls(5);
    updateLabels(scene);
    draw(scene);
}

void moveCursor(int dx, int dy) {
    int x = qBound(0, cellX(game.cursor) + dx, game.columns - 1);
    int y = qBound(0, cellY(game.cursor) + dy, game.rows - 1);
    game.cursor = index(x, y);
}

void activateCell(Scene& scene) {
    if (game.gameOver) { reset(scene); return; }

    if (game.selected < 0) {
        if (game.board[game.cursor] != Empty) game.selected = game.cursor;
        else updateLabels(scene, "This cell is empty");
        return;
    }

    if (game.board[game.cursor] != Empty) {
        game.selected = game.cursor;
        return;
    }

    if (!hasPath(game.selected, game.cursor)) {
        updateLabels(scene, "No free path to this cell");
        return;
    }

    game.board[game.cursor] = game.board[game.selected];
    game.board[game.selected] = Empty;
    game.selected = -1;

    if (removeCompletedLines() == 0) {
        spawnBalls(SpawnPerTurn);
        removeCompletedLines();
    }
    updateLabels(scene);
}

} // namespace Lines98

void lines98Start(Scene& scene) {
    Lines98::reset(scene);
}

void lines98Update(Scene& scene, float /*dt*/) {
    using namespace Lines98;
    if (!game.started) return;

    const bool left = scene.input.left;
    const bool right = scene.input.right;
    const bool up = scene.input.up;
    const bool down = scene.input.down;
    const bool space = scene.input.space;

    if (left && !game.oldLeft) moveCursor(-1, 0);
    if (right && !game.oldRight) moveCursor(1, 0);
    if (up && !game.oldUp) moveCursor(0, -1);
    if (down && !game.oldDown) moveCursor(0, 1);
    if (space && !game.oldSpace) activateCell(scene);

    game.oldLeft = left;
    game.oldRight = right;
    game.oldUp = up;
    game.oldDown = down;
    game.oldSpace = space;
    draw(scene);
}
