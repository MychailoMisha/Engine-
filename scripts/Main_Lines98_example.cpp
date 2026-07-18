// Put these calls inside the existing Main.cpp hooks.

void onStart(Scene& scene) {
    lines98Start(scene);
}

void onUpdate(Scene& scene, float dt) {
    lines98Update(scene, dt);
}
