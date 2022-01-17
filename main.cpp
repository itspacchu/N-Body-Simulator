#include<raylib.h>
#include<vector>
#include<iostream>
#include<cmath>

#define void void

const int screenWidth = 1280;
const int screenHeight = 720;

float gravity_dir = 1.0;
float time_scale = 1.0;
int MAX_PLANETS = 512;



int current_MAX_ID = 0;

typedef struct Planet
{
    Vector2 position;
    Vector2 velocity;
    float radius;
    float mass;
    Color color;
    std::string name;
    int id=0;
    bool is_deleted = false;
};

std::vector<Planet> planets;


Vector2 vectorAdd(Vector2 a, Vector2 b)
{
    return (Vector2){a.x + b.x, a.y + b.y};
}

Vector2 vectorSub(Vector2 a, Vector2 b)
{
    return (Vector2){a.x - b.x, a.y - b.y};
}

Vector2 vectorAvg(Vector2 a, Vector2 b, float w1=1.0, float w2=1.0)
{
    return (Vector2){(a.x * w1 + b.x * w2) / (w1 + w2), (a.y * w1 + b.y * w2) / (w1 + w2)};
}

Vector2 vectorMin(Vector2 a, Vector2 b)
{
    return (Vector2){std::min(a.x, b.x), std::min(a.y, b.y)};
}

Vector3 fromVector2(Vector2 v){
    return (Vector3){v.x,v.y,0};
}

float vectorLength(Vector2 v){
    return sqrt(v.x*v.x+v.y*v.y);
}


void removeID(int id)
{
    for(int i = 0; i < planets.size(); i++)
    {
        if(planets[i].id == id)
        {
            planets.erase(planets.begin() + i);
            break;
        }
    }
}


void populatePlanets(int MAX=3){
    for(int i=0;i<MAX;i++){
        Planet p;
        p.position = (Vector2){(float)i/MAX * screenWidth,(float)GetRandomValue(0,screenHeight)};
        p.velocity = (Vector2){(float)GetRandomValue(1,10),(float)GetRandomValue(1,10)};
        p.radius = (float)GetRandomValue(2,5);
        p.mass = p.radius*p.radius;
        p.color = ColorFromHSV(255*(float)i/(float)MAX,0.8,1);
        p.name = std::string("Planet ") + std::to_string(i);
        p.id = i;
        planets.push_back(p);
    }
    current_MAX_ID += MAX;
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

void OffBoundDelete(Planet p){
        if(p.position.x < -5*p.radius || p.position.x>screenWidth + 5*p.radius || p.position.y < -5*p.radius || p.position.y>screenHeight + 5*p.radius){
            removeID(p.id);
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

void MergePlanets(int indexA,int indexB){
    Planet a = planets[indexA];
    Planet b = planets[indexB];
    float mass = a.mass+b.mass;
    Vector2 pos = vectorAvg(a.position,b.position,a.mass/mass,b.mass/mass);
    Vector2 vel = vectorAvg(a.velocity,b.velocity,vectorLength(a.velocity)*a.mass/mass,vectorLength(b.velocity)*b.mass/mass);
    float radius = sqrt(mass/PI);
    Color color = (Color){(a.mass*a.color.r+b.mass*b.color.r)/mass,(a.mass*a.color.g+b.mass*b.color.g)/mass,(a.mass*a.color.b+b.mass*b.color.b)/mass,255};
    std::string name = std::string("Merged ") + a.name + " and " + b.name;
    Planet p = {pos,vel,radius,mass,color,name};
    removeID(a.id);
    removeID(b.id);
    p.id = current_MAX_ID;
    current_MAX_ID++;
    planets.push_back(p);
}


void UpdateForces(float delta){
    for(int i=0;i<planets.size();i++){
        Vector2 force = {0,0};
        for(int j=0;j<planets.size();j++){
            Planet p = planets[i];
            Planet other = planets[j];
            if(planets.size() > 1){
                OffBoundDelete(p);
                OffBoundDelete(other);
            }

            Vector2 direction = {p.position.x - other.position.x,p.position.y - other.position.y};
            float distance = vectorLength(direction);
            
            if(vectorLength(vectorSub(p.position,other.position)) > (p.mass+other.mass)){
                //DrawLineV(p.position,other.position,(Color){255 - (distance/std::max(screenHeight,screenWidth)),0,0,255});
                continue;
            }else{
                DrawLineV(p.position,other.position,(Color){255,255,255,20});
            }
            
            // crude implementation of F= Gm1m2/r^2 a^ vectorized to direction
            //planets too close then merge them
            if(p.id != other.id){ 
                if(CheckCollisionCircles(p.position,0.55*p.radius,other.position,other.radius)){
                    MergePlanets(i,j);
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
        //OffBoundMirror();
        planets[i].position.x += planets[i].velocity.x*delta*time_scale;
        planets[i].position.y += planets[i].velocity.y*delta*time_scale;
    }
}

int main(){
    populatePlanets(MAX_PLANETS);
    using RAYLIB_H::DrawTextEx;
    SetConfigFlags(FLAG_MSAA_4X_HINT);  
    InitWindow(screenWidth, screenHeight, "Planetary Chaos?");

    Shader BlurShader = LoadShader(0, "/home/pac/Documents/secretstuff/gravitational/assets/blur.fs");
    bool initPlanetMouse = false;
    RenderTexture2D pass1 = LoadRenderTexture(screenWidth,screenHeight);
    RenderTexture2D pass2 = LoadRenderTexture(screenWidth,screenHeight);
    RenderTexture2D pass3 = LoadRenderTexture(screenWidth,screenHeight);
    ClearBackground(BLACK);
    SetTargetFPS(75);
    while (!WindowShouldClose()){
        Vector2 cacheMousePos;
        if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            cacheMousePos = GetMousePosition();  
            initPlanetMouse = true;
            
        }
        if(IsMouseButtonReleased(MOUSE_LEFT_BUTTON)){
                Vector2 mousePos = GetMousePosition();
                mousePos.y = screenHeight - mousePos.y;
                cacheMousePos.y = screenHeight - cacheMousePos.y;
                Planet p;
                p.position = cacheMousePos;
                p.velocity = (Vector2){-mousePos.x + cacheMousePos.x, -mousePos.y + cacheMousePos.y};
                p.radius = 10;
                p.mass = PI*p.radius*p.radius;
                p.color = (Color){255,255,255,255};
                p.id = current_MAX_ID;
                p.name = "Planet " + std::to_string(current_MAX_ID);
                current_MAX_ID++;
                planets.push_back(p);
                initPlanetMouse = false;
                std::cout << "Added planet:" << std::to_string(p.id) << std::endl;
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
            UpdatePlanets(GetFrameTime());
            DrawPlanets(255);
        EndTextureMode();
        

        BeginDrawing();
                ClearBackground(WHITE);
                BlendMode(BLEND_ADDITIVE);
                    DrawTextureRec(pass1.texture,(Rectangle){ 0, 0, (float)pass1.texture.width, (float)pass1.texture.height },(Vector2){0,0},WHITE);
                EndBlendMode();
                DrawTextureRec(pass3.texture,(Rectangle){ 0, 0, (float)pass3.texture.width, (float)pass3.texture.height },(Vector2){0,0},(Color){255,255,255,42});
                if(initPlanetMouse){
                    DrawCircleLines(cacheMousePos.x,cacheMousePos.y,10,(Color){255,255,255,255});
                    DrawLine(cacheMousePos.x,cacheMousePos.y,GetMousePosition().x,GetMousePosition().y,(Color){255,255,255,255});
                }
                DrawText((std::string("Gravity: ") + std::to_string(gravity_dir) + (std::string(" | TimeScale: ") + std::to_string(time_scale))).c_str(),10,10,20,(Color){255,255,255,255});
        EndDrawing();

        if(IsKeyDown(KEY_RIGHT))time_scale *= 1.05;
        if(IsKeyDown(KEY_LEFT))time_scale *= 1/1.05;
        if(IsKeyPressed(KEY_SPACE))gravity_dir *= -1;
        if(IsKeyPressed(KEY_R)){
            planets.clear();
            populatePlanets(MAX_PLANETS);
        }
        // add new planet
    }
}
