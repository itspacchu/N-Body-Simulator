#include<raylib.h>
#include<vector>
#include<iostream>
#include<cmath>

#define void void

const int screenWidth = 1280;
const int screenHeight = 720;



float gravity_dir = 1.0;
float time_scale = 1.0;

typedef struct Planet
{
    Vector2 position;
    Vector2 velocity;
    float radius;
    float mass;
    Color color;
    std::string name;
};

std::vector<Planet> planets;


Vector2 vectorAdd(Vector2 a, Vector2 b)
{
    return (Vector2){a.x + b.x, a.y + b.y};
}

void populatePlanets(int MAX=3){
    for(int i=0;i<MAX;i++){
        Planet p;
        p.position = (Vector2){(float)i/MAX * screenWidth,(float)GetRandomValue(0,screenHeight)};
        p.velocity = (Vector2){(float)GetRandomValue(1,10),(float)GetRandomValue(1,10)};
        p.radius = (float)GetRandomValue(2,10);
        p.mass = PI*p.radius*p.radius;
        p.color = ColorFromHSV(255*(float)i/(float)MAX,0.8,1);
        p.name = std::string("Planet ") + std::to_string(i);
        planets.push_back(p);
    
    }
}

Vector3 fromVector2(Vector2 v){
    return (Vector3){v.x,v.y,0};
}

float vectorLength(Vector2 v){
    return sqrt(v.x*v.x+v.y*v.y);
}


void DrawPlanet(Planet p,float Alpha){
    Color c = p.color;
    c.a = Alpha;
    DrawCircleV(p.position,p.radius,c);
}

void DrawPlanets(float Alpha){
    for(Planet p : planets){
        DrawPlanet(p,Alpha);
    }
}

void OffBoundMirror(){
    for(int i=0;i<planets.size();i++){
        if(planets[i].position.x > screenWidth){
            planets[i].position.x = 0;
        }
        if(planets[i].position.x < 0){
            planets[i].position.x = screenWidth;
        }
        if(planets[i].position.y > screenHeight){
            planets[i].position.y = 0;
        }
        if(planets[i].position.y < 0){
            planets[i].position.y = screenHeight;
        }
    }
}

void UpdateForces(float delta){
    for(int i=0;i<planets.size();i++){
        Vector2 force = {0,0};
        for(int j=0;j<planets.size();j++){
            Planet p = planets[i];
            Planet other = planets[j];
            // crude implementation of F= Gm1m2/r^2 vectorized to direction
            if(p.name != other.name){ 
                Vector2 direction = {p.position.x - other.position.x,p.position.y - other.position.y};
                float distance = vectorLength(direction);
                if(distance > 150){ //hmm why
                    continue;
                }
                float forceMagnitude = 0.5*(p.mass*other.mass)/(distance*distance);
                force.x -= gravity_dir*forceMagnitude*direction.x;
                force.y -= gravity_dir*forceMagnitude*direction.y;
            }
        }
        planets[i].velocity.x += force.x*delta;
        planets[i].velocity.y += force.y*delta;
    }
}

void UpdatePlanets(float delta){
    UpdateForces(delta/10.0);
    for(int i=0;i<planets.size();i++){
        OffBoundMirror();
        planets[i].position.x += planets[i].velocity.x*delta*time_scale;
        planets[i].position.y += planets[i].velocity.y*delta*time_scale;
    }
}

int main(){
    populatePlanets(128);
    using RAYLIB_H::DrawTextEx;
    SetConfigFlags(FLAG_MSAA_4X_HINT);  
    InitWindow(screenWidth, screenHeight, "Planetary Chaos?");

    Shader BlurShader = LoadShader(0, "/home/pac/Documents/secretstuff/gravitational/assets/blur.fs");

    RenderTexture2D pass1 = LoadRenderTexture(screenWidth,screenHeight);
    RenderTexture2D pass2 = LoadRenderTexture(screenWidth,screenHeight);
    RenderTexture2D pass3 = LoadRenderTexture(screenWidth,screenHeight);
    ClearBackground(BLACK);
    SetTargetFPS(75);
    while (!WindowShouldClose()){
        if(IsKeyDown(KEY_RIGHT))time_scale *= 1.05;
        if(IsKeyDown(KEY_LEFT))time_scale *= 1/1.05;
        if(IsKeyPressed(KEY_SPACE))gravity_dir *= -1;
        if(IsKeyPressed(KEY_R)){
            planets.clear();
            populatePlanets(5);
        }

        BeginTextureMode(pass2);
            DrawPlanets(42);
        EndTextureMode();

        BeginTextureMode(pass3); 
            BeginShaderMode(BlurShader);
            DrawTextureRec(pass2.texture,(Rectangle){ 0, 0, (float)pass3.texture.width, -(float)pass3.texture.height },(Vector2){0,0},WHITE);
            EndShaderMode();
        EndTextureMode();

        BeginTextureMode(pass1);
            ClearBackground(BLACK);
            UpdatePlanets(GetFrameTime()/2.0);
            DrawPlanets(255);
        EndTextureMode();
        

        BeginDrawing();
                ClearBackground(WHITE);
                
                BlendMode(BLEND_ADDITIVE);
                    DrawTextureRec(pass1.texture,(Rectangle){ 0, 0, (float)pass1.texture.width, (float)pass1.texture.height },(Vector2){0,0},WHITE);
                EndBlendMode();
                DrawTextureRec(pass3.texture,(Rectangle){ 0, 0, (float)pass3.texture.width, (float)pass3.texture.height },(Vector2){0,0},(Color){255,255,255,42});
                DrawText((std::string("Gravity: ") + std::to_string(gravity_dir) + (std::string(" | TimeScale: ") + std::to_string(time_scale))).c_str(),10,10,20,(Color){255,255,255,255});
        EndDrawing();
    }
}
