#include "geGL/Program.h"
#include "geGL/Shader.h"
#include<iostream>

#include<SDL3/SDL.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>
#include <memory>

using namespace ge::gl;

int main(int argc,char*argv[]){

  auto window = SDL_CreateWindow("MULe",1024,768,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);

  ge::gl::init();

  //GLSL vertex shader
  auto const vsSrc = R".(
  #version 460

  void main(){
    if(gl_VertexID==0)gl_Position = vec4(0,0,0,1);
    if(gl_VertexID==1)gl_Position = vec4(1,0,0,1);
    if(gl_VertexID==2)gl_Position = vec4(0,1,0,1);
  }

  ).";

  auto const fsSrc = R".(
  #version 460

  out vec4 fColor;
  void main(){
    fColor = vec4(0,0,1,1);
  }

  ).";

  auto vs = std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER,vsSrc);
  auto fs = std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER,fsSrc);
  auto prg = std::make_shared<ge::gl::Program>(vs,fs);

  bool running = true;
  while(running){//main loop
  
    SDL_Event event;
    while(SDL_PollEvent(&event)){//event loop
      if(event.type == SDL_EVENT_QUIT)running = false;
    }

    //rendering
    //
    glClearColor(1,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glPointSize(10);

    prg->use();
    glDrawArrays(GL_TRIANGLES,0,3);

    SDL_GL_SwapWindow(window); // double buffering
  }

  SDL_GL_DestroyContext(context);

  SDL_DestroyWindow(window);
  return 0;
}
