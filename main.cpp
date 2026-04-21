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
    mat4 model = mat4(1);
    model[0][0] =  cos(iTime);
    model[0][1] =  sin(iTime);
    model[1][0] = -sin(iTime);
    model[1][1] =  cos(iTime);


    if(gl_VertexID==0){gl_Position = model*vec4(-1,-1,0,1);vCoord = vec2(0,0);}
    if(gl_VertexID==1){gl_Position = model*vec4(+1,-1,0,1);vCoord = vec2(1,0);}
    if(gl_VertexID==2){gl_Position = model*vec4(-1,+1,0,1);vCoord = vec2(0,1);}
    if(gl_VertexID==3){gl_Position = model*vec4(+1,+1,0,1);vCoord = vec2(1,1);}
  }

  ).";

  auto const fsSrc = R".(
  #version 460

  layout(binding=0)uniform sampler2D tex;

  in vec2 vCoord;

  out vec4 fColor;
  void main(){
    fColor = texture(tex,vec2(vCoord.x,1-vCoord.y));
  }

  ).";


  auto vs = std::make_shared<ge::gl::Shader>(GL_VERTEX_SHADER,vsSrc);
  auto fs = std::make_shared<ge::gl::Shader>(GL_FRAGMENT_SHADER,fsSrc);
  auto prg = std::make_shared<ge::gl::Program>(vs,fs);

  GLuint tex;
  glCreateTextures(GL_TEXTURE_2D,1,&tex);
  glTextureStorage2D(tex,1,GL_RGB8,width,height);

  glTextureParameteri(tex, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
  glTextureParameteri(tex, GL_TEXTURE_MAG_FILTER,GL_NEAREST);

  float iTime = 0.f;
  bool running = true;
  while(running){//main loop
  
    SDL_Event event;
    while(SDL_PollEvent(&event)){//event loop
      if(event.type == SDL_EVENT_QUIT)running = false;
    }
    //rendering

    video.read(bgr_frame);
    glTextureSubImage2D(tex,0,0,0,width,height,GL_BGR,GL_UNSIGNED_BYTE,bgr_frame.data);

    glClearColor(0.1,0.1,0.1,1);
    glClear(GL_COLOR_BUFFER_BIT);


    prg->use();
    prg->set1f("iTime",iTime);
    iTime+=0.01;
    glBindTextureUnit(0,tex);
    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    SDL_GL_SwapWindow(window); // double buffering
  }

  SDL_GL_DestroyContext(context);

  SDL_DestroyWindow(window);
  return 0;
}
