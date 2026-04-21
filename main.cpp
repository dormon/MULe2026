#include<geGL/DebugMessage.h>
#include<geGL/Program.h>
#include<geGL/Shader.h>
#include<iostream>

#include<SDL3/SDL.h>
#include<geGL/geGL.h>
#include<geGL/StaticCalls.h>
#include <memory>

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>

using namespace ge::gl;

int main(int argc,char*argv[]){
  cv::VideoCapture video;
  video.open("../mtm.mp4");
  cv::Mat bgr_frame;

  auto width  = video.get(cv::CAP_PROP_FRAME_WIDTH);
  auto height = video.get(cv::CAP_PROP_FRAME_HEIGHT);


  auto window = SDL_CreateWindow("MULe",width,height,SDL_WINDOW_OPENGL);
  auto context = SDL_GL_CreateContext(window);

  ge::gl::init();

  ge::gl::setHighDebugMessage();

  //GLSL vertex shader
  auto const vsSrc = R".(
  #version 460

  out vec2 vCoord;
  
  uniform float iTime;

  void main(){


    if(gl_VertexID==0){gl_Position = vec4(-1,-1,0,1);vCoord = vec2(0,0);}
    if(gl_VertexID==1){gl_Position = vec4(+1,-1,0,1);vCoord = vec2(1,0);}
    if(gl_VertexID==2){gl_Position = vec4(-1,+1,0,1);vCoord = vec2(0,1);}
    if(gl_VertexID==3){gl_Position = vec4(+1,+1,0,1);vCoord = vec2(1,1);}
  }

  ).";

  auto const fsSrc = R".(
  #version 460

  layout(binding=0)uniform sampler2D tex;

  uniform int enableEffect;
  uniform float iTime;

  in vec2 vCoord;

  out vec4 fColor;
  void main(){
    vec4 color = texture(tex,vec2(vCoord.x,1-vCoord.y));

    // effect
    if(enableEffect==1){

      vec4 ca = texture(tex,vec2(vCoord.x,1-vCoord.y));
      vec4 cb = texture(tex,vec2(vCoord.x+0.01*cos(iTime),1-vCoord.y+0.01*sin(iTime)));
      color = cb-ca;
    }

    fColor = color;
  }

  ).";


  auto vs = std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER,vsSrc);
  auto fs = std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER,fsSrc);
  auto prg = std::make_shared<ge::gl::Program>(vs,fs);
  prg->setNonexistingUniformWarning(false);

  GLuint tex;
  glCreateTextures(GL_TEXTURE_2D,1,&tex);
  glTextureStorage2D(tex,1,GL_RGB8,width,height);

  glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER,GL_NEAREST);

  float iTime = 0.f;
  bool running = true;
  bool readVideo = true;
  bool enableEffect = false;
  while(running){//main loop
  
    SDL_Event event;
    while(SDL_PollEvent(&event)){//event loop
      if(event.type == SDL_EVENT_QUIT)running = false;
      if(event.type == SDL_EVENT_KEY_DOWN){
        if(event.key.key == SDLK_SPACE)readVideo = !readVideo;
        if(event.key.key == SDLK_E    )enableEffect = !enableEffect;
      }
    }
    //rendering

    if(readVideo){
      video.read(bgr_frame);
      glTextureSubImage2D(tex,0,0,0,width,height,GL_BGR,GL_UNSIGNED_BYTE,bgr_frame.data);
    }

    glClearColor(0.1,0.1,0.1,1);
    glClear(GL_COLOR_BUFFER_BIT);


    prg->use();
    prg->set1f("iTime",iTime);
    prg->set1i("enableEffect",enableEffect);
    iTime+=0.01;
    glBindTextureUnit(0,tex);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    SDL_GL_SwapWindow(window); // double buffering
  }

  SDL_GL_DestroyContext(context);

  SDL_DestroyWindow(window);
  return 0;
}
