#include "geGL/DebugMessage.h"
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

  ge::gl::setHighDebugMessage();

  //GLSL vertex shader
  auto const vsSrc = R".(
  #version 460

  out vec2 vCoord;

  void main(){
    if(gl_VertexID==0){gl_Position = vec4(-1,-1,0,2);vCoord = vec2(0,0);}
    if(gl_VertexID==1){gl_Position = vec4(+1,-1,0,2);vCoord = vec2(1,0);}
    if(gl_VertexID==2){gl_Position = vec4(-1,+1,0,2);vCoord = vec2(0,1);}
    if(gl_VertexID==3){gl_Position = vec4(+1,+1,0,2);vCoord = vec2(1,1);}
  }

  ).";

  auto const fsSrc = R".(
  #version 460

  layout(binding=0)uniform sampler2D tex;

  in vec2 vCoord;

  out vec4 fColor;
  void main(){
    fColor = texture(tex,vCoord);
  }

  ).";

  auto vs = std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER,vsSrc);
  auto fs = std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER,fsSrc);
  auto prg = std::make_shared<ge::gl::Program>(vs,fs);

  GLuint tex;
  glCreateTextures(GL_TEXTURE_2D,1,&tex);
  glTextureStorage2D(tex,1,GL_RGBA8,2,2);

  uint8_t data[] = {
    0,255,0,255,
    255,0,0,255,
    255,0,0,255,
    0,255,0,255,
  };
  glTextureSubImage2D(tex,0,0,0,2,2,GL_RGBA,GL_UNSIGNED_BYTE,data);
  glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER,GL_NEAREST);

  bool running = true;
  while(running){//main loop
  
    SDL_Event event;
    while(SDL_PollEvent(&event)){//event loop
      if(event.type == SDL_EVENT_QUIT)running = false;
    }

    //rendering
    //
    glClearColor(0.1,0.1,0.1,1);
    glClear(GL_COLOR_BUFFER_BIT);

    glPointSize(10);

    prg->use();
    glBindTextureUnit(0,tex);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    SDL_GL_SwapWindow(window); // double buffering
  }

  SDL_GL_DestroyContext(context);

  SDL_DestroyWindow(window);
  return 0;
}
