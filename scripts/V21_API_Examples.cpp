// Put these calls inside your existing onStart/onUpdate or button logic.
// Do not add another onStart/onUpdate pair if Main.cpp already owns them.

void playHapticFeedback(Scene& scene) {
    scene.vibrate(120);
}

void saveCurrentProgress(Scene& scene) {
    scene.saveGame("MainSave");
}

void restoreProgress(Scene& scene) {
    scene.loadGame("MainSave");
}
