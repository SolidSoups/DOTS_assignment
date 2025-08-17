#pragma once
#include "Debug.h"
#include <cstring>
#include <algorithm>
#include <string>
#include <iostream>
#include <vector>


class FrameTime{
private:
  const int MAX_FRAMES = 1000;
  const float REFRESH_RATE = 0.5f ;


  float frameTimes[1000];
  std::vector<float> frameTimesNew;
  int currentIndex = 0;
  float acc = 0.f;

public: 
  float onepercentlow = 0.0f;
  FrameTime() : currentIndex(0) {
    memset(frameTimes, 0, sizeof(float)*MAX_FRAMES);
  }

  void Update(float dt){
    frameTimes[currentIndex] = dt * 1000;
    frameTimesNew.push_back(dt*1000);
    currentIndex = (currentIndex + 1) % MAX_FRAMES;

    if(frameTimesNew.size() >= MAX_FRAMES)
      frameTimesNew.erase(frameTimesNew.begin());

    acc += dt;
    if(acc >= REFRESH_RATE){
      acc = 0.f;
      Update1PercentLows();
    }
  }

private:
  void Update1PercentLows(){

    std::vector<float> tempVct = frameTimesNew;
    std::sort(tempVct.begin(), tempVct.end());

    int worst_count = frameTimesNew.size() * 0.1f;
    float totalWorst = 0.0f;
    for(int i=0; i<worst_count; ++i){
      totalWorst += tempVct[i];
    }

    float avgWorst = totalWorst / worst_count;
    onepercentlow = avgWorst > 0.0f ? avgWorst : 0.0f;
  }
};
