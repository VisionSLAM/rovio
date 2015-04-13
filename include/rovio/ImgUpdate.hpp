/*
* Copyright (c) 2014, Autonomous Systems Lab
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
* * Redistributions of source code must retain the above copyright
* notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of the Autonomous Systems Lab, ETH Zurich nor the
* names of its contributors may be used to endorse or promote products
* derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/

#ifndef ROVIO_IMGUPDATE_HPP_
#define ROVIO_IMGUPDATE_HPP_

#include "kindr/rotations/RotationEigen.hpp"
#include <Eigen/Dense>
#include "lightweight_filtering/Update.hpp"
#include "lightweight_filtering/State.hpp"
#include "rovio/FilterStates.hpp"
#include <cv_bridge/cv_bridge.h>
#include "rovio/Camera.hpp"
#include "rovio/PixelOutputCF.hpp"

namespace rot = kindr::rotations::eigen_impl;

namespace rovio {

template<typename STATE>
class ImgInnovation: public LWF::State<LWF::ArrayElement<LWF::VectorElement<2>,STATE::nMax_>>{
 public:
  typedef LWF::State<LWF::ArrayElement<LWF::VectorElement<2>,STATE::nMax_>> Base;
  using Base::E_;
  static constexpr unsigned int _nor = 0;
  ImgInnovation(){
    static_assert(_nor+1==E_,"Error with indices");
    this->template getName<_nor>() = "nor";
  };
  ~ImgInnovation(){};
};
template<typename STATE>
class ImgUpdateMeasAuxiliary: public LWF::AuxiliaryBase<ImgUpdateMeasAuxiliary<STATE>>{
 public:
  ImgUpdateMeasAuxiliary(){
    imgTime_ = 0.0;
  };
  ~ImgUpdateMeasAuxiliary(){};
  ImagePyramid<STATE::nLevels_> pyr_;
  double imgTime_;
};
template<typename STATE>
class ImgUpdateMeas: public LWF::State<ImgUpdateMeasAuxiliary<STATE>>{
 public:
  typedef LWF::State<ImgUpdateMeasAuxiliary<STATE>> Base;
  using Base::E_;
  static constexpr unsigned int _aux = 0;
  ImgUpdateMeas(){
    static_assert(_aux+1==E_,"Error with indices");
  };
  ~ImgUpdateMeas(){};
};
template<typename STATE>
class ImgUpdateNoise: public LWF::State<LWF::ArrayElement<LWF::VectorElement<2>,STATE::nMax_>>{
 public:
  typedef LWF::State<LWF::ArrayElement<LWF::VectorElement<2>,STATE::nMax_>> Base;
  using Base::E_;
  static constexpr unsigned int _nor = 0;
  ImgUpdateNoise(){
    static_assert(_nor+1==E_,"Error with indices");
    this->template getName<_nor>() = "nor";
  };
  ~ImgUpdateNoise(){};
};
template<typename STATE>
class ImgOutlierDetection: public LWF::OutlierDetection<LWF::ODEntry<ImgInnovation<STATE>::template getId<ImgInnovation<STATE>::_nor>(),2,STATE::nMax_>>{
};

template<typename FILTERSTATE>
class ImgUpdate: public LWF::Update<ImgInnovation<typename FILTERSTATE::mtState>,FILTERSTATE,ImgUpdateMeas<typename FILTERSTATE::mtState>,ImgUpdateNoise<typename FILTERSTATE::mtState>,
                                    ImgOutlierDetection<typename FILTERSTATE::mtState>,false>{
 public:
  typedef LWF::Update<ImgInnovation<typename FILTERSTATE::mtState>,FILTERSTATE,ImgUpdateMeas<typename FILTERSTATE::mtState>,ImgUpdateNoise<typename FILTERSTATE::mtState>,
                      ImgOutlierDetection<typename FILTERSTATE::mtState>,false> Base;
  using typename Base::eval;
  using Base::doubleRegister_;
  using Base::intRegister_;
  using Base::boolRegister_;
  using Base::updnoiP_;
  using Base::useSpecialLinearizationPoint_;
  typedef typename Base::mtState mtState;
  typedef typename Base::mtFilterState mtFilterState;
  typedef typename Base::mtInnovation mtInnovation;
  typedef typename Base::mtMeas mtMeas;
  typedef typename Base::mtNoise mtNoise;
  typedef typename Base::mtJacInput mtJacInput;
  typedef typename Base::mtJacNoise mtJacNoise;
  typedef typename Base::mtOutlierDetection mtOutlierDetection;
  M3D initCovFeature_;
  double initDepth_;
  Camera* mpCamera_;
  PixelOutputCF<typename FILTERSTATE::mtState> pixelOutputCF_;
  PixelOutput pixelOutput_;
  typename PixelOutputCF<typename FILTERSTATE::mtState>::mtOutputCovMat pixelOutputCov_;
  int startLevel_;
  int endLevel_;
  double startDetectionTh_;
  int nDetectionBuckets_;
  int detectionThreshold_;
  double scoreDetectionExponent_;
  double penaltyDistance_;
  double zeroDistancePenalty_;
  double trackingLocalRange_,trackingLocalVisibilityRange_;
  double trackingUpperBound_,trackingLowerBound_;
  double minTrackedAndFreeFeatures_;
  double minRelativeSTScore_;
  double minAbsoluteSTScore_;
  bool doPatchWarping_;
  bool useDirectMethod_;
  bool doDrawTracks_;
  bool doDrawVirtualHorizon_;
  ImgUpdate(){
    mpCamera_ = nullptr;
    initCovFeature_.setIdentity();
    initDepth_ = 0;
    startLevel_ = 3;
    endLevel_ = 1;
    startDetectionTh_ = 0.9;
    nDetectionBuckets_ = 100;
    detectionThreshold_ = 10; // TODO: register
    scoreDetectionExponent_ = 0.5;
    penaltyDistance_ = 20;
    zeroDistancePenalty_ = nDetectionBuckets_*1.0;
    doPatchWarping_ = true;
    useDirectMethod_ = true;
    doDrawTracks_ = true;
    doDrawVirtualHorizon_ = true; // TODO: register
    trackingLocalRange_ = 20;
    trackingLocalVisibilityRange_ = 200;
    trackingUpperBound_ = 0.9;
    trackingLowerBound_ = 0.1;
    minTrackedAndFreeFeatures_ = 0.5;
    minRelativeSTScore_ = 0.2;
    minAbsoluteSTScore_ = 0.2;
    doubleRegister_.registerDiagonalMatrix("initCovFeature",initCovFeature_);
    doubleRegister_.registerScalar("initDepth",initDepth_);
    doubleRegister_.registerScalar("startDetectionTh",startDetectionTh_);
    doubleRegister_.registerScalar("scoreDetectionExponent",scoreDetectionExponent_);
    doubleRegister_.registerScalar("penaltyDistance",penaltyDistance_);
    doubleRegister_.registerScalar("zeroDistancePenalty",zeroDistancePenalty_);
    doubleRegister_.registerScalar("trackingLocalRange",trackingLocalRange_);
    doubleRegister_.registerScalar("trackingLocalVisibilityRange",trackingLocalVisibilityRange_);
    doubleRegister_.registerScalar("trackingUpperBound",trackingUpperBound_);
    doubleRegister_.registerScalar("trackingLowerBound",trackingLowerBound_);
    doubleRegister_.registerScalar("minTrackedAndFreeFeatures",minTrackedAndFreeFeatures_);
    doubleRegister_.registerScalar("minRelativeSTScore",minRelativeSTScore_);
    doubleRegister_.registerScalar("minAbsoluteSTScore",minAbsoluteSTScore_);
    intRegister_.registerScalar("startLevel",startLevel_);
    intRegister_.registerScalar("endLevel",endLevel_);
    intRegister_.registerScalar("nDetectionBuckets",nDetectionBuckets_);
    boolRegister_.registerScalar("doPatchWarping",doPatchWarping_);
    boolRegister_.registerScalar("useDirectMethod",useDirectMethod_);
    boolRegister_.registerScalar("doDrawTracks",doDrawTracks_);
    int ind;
    for(int i=0;i<FILTERSTATE::mtState::nMax_;i++){
      ind = mtNoise::template getId<mtNoise::_nor>(i);
      doubleRegister_.removeScalarByVar(updnoiP_(ind,ind));
      doubleRegister_.removeScalarByVar(updnoiP_(ind+1,ind+1));
      doubleRegister_.registerScalar("UpdateNoise.nor",updnoiP_(ind,ind));
      doubleRegister_.registerScalar("UpdateNoise.nor",updnoiP_(ind+1,ind+1));
    }
  };
  ~ImgUpdate(){};
  void refreshProperties(){
    useSpecialLinearizationPoint_ = useDirectMethod_;
  };
  void setCamera(Camera* mpCamera){
    mpCamera_ = mpCamera;
    pixelOutputCF_.setCamera(mpCamera);
  }
  void eval(mtInnovation& y, const mtState& state, const mtMeas& meas, const mtNoise noise, double dt = 0.0) const{
    for(unsigned int i=0;i<state.nMax_;i++){
      if(state.template get<mtState::_aux>().useInUpdate_[i]){
        if(useDirectMethod_){
          y.template get<mtInnovation::_nor>(i) = state.template get<mtState::_aux>().b_red_[i]+noise.template get<mtNoise::_nor>(i);
        } else {
          state.template get<mtState::_nor>(i).boxMinus(state.template get<mtState::_aux>().bearingMeas_[i],y.template get<mtInnovation::_nor>(i)); // 0 = m - m_meas + n
          y.template get<mtInnovation::_nor>(i) += noise.template get<mtNoise::_nor>(i);
        }
      } else {
        y.template get<mtInnovation::_nor>(i) = noise.template get<mtNoise::_nor>(i);
      }
    }
  }
  void jacInput(mtJacInput& F, const mtState& state, const mtMeas& meas, double dt = 0.0) const{
    F.setZero();
    cv::Point2f c_temp;
    Eigen::Matrix2d c_J;
    for(unsigned int i=0;i<state.nMax_;i++){
      if(state.template get<mtState::_aux>().useInUpdate_[i]){
        if(useDirectMethod_){
          mpCamera_->bearingToPixel(state.template get<mtState::_nor>(i),c_temp,c_J);
          F.template block<2,2>(mtInnovation::template getId<mtInnovation::_nor>(i),mtState::template getId<mtState::_nor>(i)) = -state.template get<mtState::_aux>().A_red_[i]*c_J;
        } else {
          F.template block<2,2>(mtInnovation::template getId<mtInnovation::_nor>(i),mtState::template getId<mtState::_nor>(i)) =
                state.template get<mtState::_aux>().bearingMeas_[i].getN().transpose()
                *-LWF::NormalVectorElement::getRotationFromTwoNormalsJac(state.template get<mtState::_nor>(i),state.template get<mtState::_aux>().bearingMeas_[i])
                *state.template get<mtState::_nor>(i).getM();
        }
      }
    }
  }
  void jacNoise(mtJacNoise& G, const mtState& state, const mtMeas& meas, double dt = 0.0) const{
    G.setZero();
    for(unsigned int i=0;i<FILTERSTATE::mtState::nMax_;i++){
      G.template block<2,2>(mtInnovation::template getId<mtInnovation::_nor>(i),mtNoise::template getId<mtNoise::_nor>(i)) = Eigen::Matrix2d::Identity();
    }
  }
  void preProcess(mtFilterState& filterState, const mtMeas& meas, const int s = 0){
    assert(filterState.t_ == meas.template get<mtMeas::_aux>().imgTime_);
    typename mtFilterState::mtState& state = filterState.state_;
    typename mtFilterState::mtFilterCovMat& cov = filterState.cov_;
    cvtColor(meas.template get<mtMeas::_aux>().pyr_.imgs_[0], filterState.img_, CV_GRAY2RGB);
    filterState.imgTime_ = filterState.t_;
    filterState.imageCounter_++;
    filterState.patchDrawing_ = cv::Mat::zeros(mtState::nMax_*pow(2,mtState::nLevels_-1),mtState::nMax_*pow(2,mtState::nLevels_-1),CV_8UC1); // TODO

    MultilevelPatchFeature<mtState::nLevels_,mtState::patchSize_>* mpFeature;
    Eigen::Vector2d vec2;
    for(unsigned int i=0;i<mtState::nMax_;i++){
      if(filterState.mlps_.isValid_[i]){
        mpFeature = &filterState.mlps_.features_[i];
        // Data handling stuff
        mpFeature->set_nor(state.template get<mtState::_nor>(i));
        mpFeature->increaseStatistics(filterState.t_);

        // Check if prediction in frame
        mpFeature->status_.inFrame_ = isMultilevelPatchInFrame(*mpFeature,meas.template get<mtMeas::_aux>().pyr_,startLevel_,false,doPatchWarping_);
        if(mpFeature->status_.inFrame_){
          pixelOutputCF_.setIndex(i);
          pixelOutputCov_ = pixelOutputCF_.transformCovMat(state,cov);
          mpFeature->log_prediction_.c_ = mpFeature->get_c();
          mpFeature->log_prediction_.setSigmaFromCov(pixelOutputCov_);
          mpFeature->set_bearingCorners(state.template get<mtState::_aux>().bearingCorners_[i]);
          const PixelCorners& pixelCorners = mpFeature->get_pixelCorners();
          mpFeature->log_predictionC0_.c_ = mpFeature->get_c() - 4*pixelCorners[0] - 4*pixelCorners[1];
          mpFeature->log_predictionC1_.c_ = mpFeature->get_c() + 4*pixelCorners[0] - 4*pixelCorners[1];
          mpFeature->log_predictionC2_.c_ = mpFeature->get_c() - 4*pixelCorners[0] + 4*pixelCorners[1];
          mpFeature->log_predictionC3_.c_ = mpFeature->get_c() + 4*pixelCorners[0] + 4*pixelCorners[1];

          // Search patch // TODO: do adaptive
          align2DComposed(*mpFeature,meas.template get<mtMeas::_aux>().pyr_,startLevel_,endLevel_,startLevel_-endLevel_,doPatchWarping_);
          if(mpFeature->status_.matchingStatus_ == FOUND) mpFeature->log_meas_.c_ = mpFeature->get_c();

          // Add as measurement
          if(useDirectMethod_){
            vec2.setZero();
            if(mpFeature->status_.matchingStatus_ == FOUND){
              mpFeature->get_nor().boxMinus(state.template get<mtState::_nor>(i),vec2);
              if((vec2.transpose()*cov.template block<2,2>(mtState::template getId<mtState::_nor>(i),mtState::template getId<mtState::_nor>(i)).inverse()*vec2)(0,0) > 5.886){ // TODO: param
                mpFeature->set_nor(state.template get<mtState::_nor>(i));
                vec2.setZero();
              }
            }
            filterState.difVecLin_.template block<2,1>(mtState::template getId<mtState::_nor>(i),0) = vec2;
            if(getLinearAlignEquationsReduced(*mpFeature,meas.template get<mtMeas::_aux>().pyr_,endLevel_,startLevel_,doPatchWarping_,
                                              state.template get<mtState::_aux>().A_red_[i],state.template get<mtState::_aux>().b_red_[i])){
              state.template get<mtState::_aux>().useInUpdate_[i] = true;
            } else {
              state.template get<mtState::_aux>().useInUpdate_[i] = false;
            }
          } else {
            if(mpFeature->status_.matchingStatus_ == FOUND){
              state.template get<mtState::_aux>().useInUpdate_[i] = true;
              state.template get<mtState::_aux>().bearingMeas_[i] = mpFeature->get_nor();
            } else {
              state.template get<mtState::_aux>().useInUpdate_[i] = false;
            }
          }
        } else {
          state.template get<mtState::_aux>().useInUpdate_[i] = false;
        }
      } else {
        state.template get<mtState::_aux>().useInUpdate_[i] = false;
      }
    }
  };
  void postProcess(mtFilterState& filterState, const mtMeas& meas, const mtOutlierDetection& outlierDetection, const int s = 0){
    // Temps
    typename mtFilterState::mtState& state = filterState.state_;
    typename mtFilterState::mtFilterCovMat& cov = filterState.cov_;
    float averageScore;
    int countTracked;
    MultilevelPatchFeature<mtState::nLevels_,mtState::patchSize_>* mpFeature2; // TODO: rename
    MultilevelPatchFeature<mtState::nLevels_,mtState::patchSize_> testFeature2; // TODO: rename
    int requiredFreeFeature;
    double removalFactor;
    int featureIndex;

    countTracked = 0;
    for(unsigned int i=0;i<mtState::nMax_;i++){
      if(filterState.mlps_.isValid_[i]){
        mpFeature2 = &filterState.mlps_.features_[i];
        if(mpFeature2->status_.inFrame_){
          // Handle Status
          mpFeature2->set_nor(state.template get<mtState::_nor>(i));
          mpFeature2->log_current_.c_ = mpFeature2->get_c();
          pixelOutputCF_.setIndex(i);
          pixelOutputCov_ = pixelOutputCF_.transformCovMat(state,cov);
          mpFeature2->log_current_.setSigmaFromCov(pixelOutputCov_);
          if(state.template get<mtState::_aux>().useInUpdate_[i]){
            if(!outlierDetection.isOutlier(i)){
              mpFeature2->status_.trackingStatus_ = TRACKED;
              countTracked++;
            } else {
              mpFeature2->status_.trackingStatus_ = FAILED;
            }
          }

          // Extract feature patches
          if(mpFeature2->status_.trackingStatus_ == TRACKED){
            if(isMultilevelPatchInFrame(*mpFeature2,meas.template get<mtMeas::_aux>().pyr_,startLevel_,true,false)){
              testFeature2.set_c(mpFeature2->get_c());
              extractMultilevelPatchFromImage(testFeature2,meas.template get<mtMeas::_aux>().pyr_,startLevel_,true,false);
              testFeature2.computeMultilevelShiTomasiScore();
              if(testFeature2.s_ >= static_cast<float>(minAbsoluteSTScore_) || testFeature2.s_ >= static_cast<float>(minRelativeSTScore_)*mpFeature2->s_){
                extractMultilevelPatchFromImage(*mpFeature2,meas.template get<mtMeas::_aux>().pyr_,startLevel_,true,false);
                state.template get<mtState::_aux>().bearingCorners_[i] = mpFeature2->get_bearingCorners();
              }
            }
          }

          // Drawing
          if(mpFeature2->status_.inFrame_ && doDrawTracks_){
            mpFeature2->log_prediction_.draw(filterState.img_,cv::Scalar(0,175,175));
            mpFeature2->log_predictionC0_.drawLine(filterState.img_,mpFeature2->log_predictionC1_,cv::Scalar(0,175,175),1);
            mpFeature2->log_predictionC0_.drawLine(filterState.img_,mpFeature2->log_predictionC2_,cv::Scalar(0,175,175),1);
            mpFeature2->log_predictionC3_.drawLine(filterState.img_,mpFeature2->log_predictionC1_,cv::Scalar(0,175,175),1);
            mpFeature2->log_predictionC3_.drawLine(filterState.img_,mpFeature2->log_predictionC2_,cv::Scalar(0,175,175),1);
            if(mpFeature2->status_.trackingStatus_ == TRACKED){
              mpFeature2->log_current_.drawLine(filterState.img_,mpFeature2->log_prediction_,cv::Scalar(0,255,0));
              mpFeature2->log_current_.draw(filterState.img_,cv::Scalar(0, 255, 0));
              mpFeature2->log_current_.drawText(filterState.img_,std::to_string(mpFeature2->totCount_),cv::Scalar(0,255,0));
            } else if(mpFeature2->status_.trackingStatus_ == FAILED){
              mpFeature2->log_current_.draw(filterState.img_,cv::Scalar(0, 0, 255));
              mpFeature2->log_current_.drawText(filterState.img_,std::to_string(mpFeature2->countTrackingStatistics(FAILED,trackingLocalRange_)),cv::Scalar(0,0,255));
            } else {
              mpFeature2->log_current_.draw(filterState.img_,cv::Scalar(0,255,255));
              mpFeature2->log_current_.drawText(filterState.img_,std::to_string(mpFeature2->countTrackingStatistics(NOTTRACKED,trackingLocalVisibilityRange_)),cv::Scalar(0,255,255));
            }
          }
        }
      }
    }

    // Remove bad feature
    averageScore = filterState.mlps_.getAverageScore();
    for(unsigned int i=0;i<mtState::nMax_;i++){
      if(filterState.mlps_.isValid_[i]){
        mpFeature2 = &filterState.mlps_.features_[i];
        if(!mpFeature2->isGoodFeature(trackingLocalRange_,trackingLocalVisibilityRange_,trackingUpperBound_,trackingLowerBound_)){
          //          || fManager.features_[ind].s_ < static_cast<float>(minAbsoluteSTScore_) + static_cast<float>(minRelativeSTScore_)*averageScore){ //TODO: debug and fix
          filterState.mlps_.isValid_[i] = false;
          filterState.removeFeature(i);
        }
      }
    }

    // Check if enough free features
    requiredFreeFeature = mtState::nMax_*minTrackedAndFreeFeatures_-countTracked;
    removalFactor = 1.1; // TODO: param
    featureIndex = 0;
    while((int)(mtState::nMax_) - (int)(filterState.mlps_.getValidCount()) < requiredFreeFeature){
      if(filterState.mlps_.isValid_[featureIndex]){
        mpFeature2 = &filterState.mlps_.features_[featureIndex];
        if(mpFeature2->status_.trackingStatus_ != TRACKED &&
          !mpFeature2->isGoodFeature(trackingLocalRange_,trackingLocalVisibilityRange_,trackingUpperBound_*removalFactor,trackingLowerBound_*removalFactor)){ // TODO: improve
          filterState.mlps_.isValid_[featureIndex] = false;
          filterState.removeFeature(featureIndex);
        }
      }
      featureIndex++;
      if(featureIndex == mtState::nMax_){
        featureIndex = 0;
        removalFactor = removalFactor*1.1; // TODO: param
      }
    }

    // Get new features
    averageScore = filterState.mlps_.getAverageScore();
    if(filterState.mlps_.getValidCount() < startDetectionTh_*mtState::nMax_){
      std::list<cv::Point2f> candidates;
      ROS_DEBUG_STREAM(" Adding keypoints");
      const double t1 = (double) cv::getTickCount();
      for(int l=endLevel_;l<=startLevel_;l++){
        detectFastCorners(meas.template get<mtMeas::_aux>().pyr_,candidates,l,detectionThreshold_);
      }
      const double t2 = (double) cv::getTickCount();
      ROS_DEBUG_STREAM(" == Detected " << candidates.size() << " on levels " << endLevel_ << "-" << startLevel_ << " (" << (t2-t1)/cv::getTickFrequency()*1000 << " ms)");
      pruneCandidates(filterState.mlps_,candidates);
      const double t3 = (double) cv::getTickCount();
      ROS_DEBUG_STREAM(" == Selected " << candidates.size() << " candidates (" << (t3-t2)/cv::getTickFrequency()*1000 << " ms)");
      std::unordered_set<unsigned int> newSet = addBestCandidates(filterState.mlps_,candidates,meas.template get<mtMeas::_aux>().pyr_,filterState.t_,
                                                                  endLevel_,startLevel_,mtState::nMax_-filterState.mlps_.getValidCount(),nDetectionBuckets_, scoreDetectionExponent_,
                                                                  penaltyDistance_, zeroDistancePenalty_,false,0.0);
      const double t4 = (double) cv::getTickCount();
      ROS_DEBUG_STREAM(" == Got " << filterState.mlps_.getValidCount() << " after adding " << newSet.size() << " features (" << (t4-t3)/cv::getTickFrequency()*1000 << " ms)");
      for(auto it = newSet.begin();it != newSet.end();++it){
        filterState.mlps_.features_[*it].setCamera(mpCamera_);
        filterState.mlps_.features_[*it].status_.inFrame_ = true;
        filterState.mlps_.features_[*it].status_.matchingStatus_ = FOUND;
        filterState.mlps_.features_[*it].status_.trackingStatus_ = TRACKED;
        filterState.initializeFeatureState(*it,filterState.mlps_.features_[*it].get_nor().getVec(),initDepth_,initCovFeature_);
        state.template get<mtState::_aux>().bearingCorners_[*it] = filterState.mlps_.features_[*it].get_bearingCorners();
      }
    }
    for(unsigned int i=0;i<mtState::nMax_;i++){
      if(filterState.mlps_.isValid_[i] && filterState.mlps_.features_[i].status_.inFrame_){
        filterState.mlps_.features_[i].log_previous_.c_ = filterState.mlps_.features_[i].get_c();
      }
    }
    if (doDrawVirtualHorizon_){
      drawVirtualHorizon(filterState);
    }
  };
  void drawVirtualHorizon(mtFilterState& filterState){
    typename mtFilterState::mtState& state = filterState.state_;
    cv::rectangle(filterState.img_,cv::Point2f(0,0),cv::Point2f(82,92),cv::Scalar(50,50,50),-1,8,0);
    cv::rectangle(filterState.img_,cv::Point2f(0,0),cv::Point2f(80,90),cv::Scalar(100,100,100),-1,8,0);
    cv::putText(filterState.img_,std::to_string(filterState.imageCounter_),cv::Point2f(5,85),cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255,0,0));
    cv::Point2f rollCenter = cv::Point2f(40,40);
    cv::Scalar rollColor1(50,50,50);
    cv::Scalar rollColor2(200,200,200);
    cv::Scalar rollColor3(120,120,120);
    cv::circle(filterState.img_,rollCenter,32,rollColor1,-1,8,0);
    cv::circle(filterState.img_,rollCenter,30,rollColor2,-1,8,0);
    Eigen::Vector3d Vg = (state.template get<mtState::_vea>()*state.template get<mtState::_att>().inverted()).rotate(Eigen::Vector3d(0,0,-1));
    double roll = atan2(Vg(1),Vg(0))-0.5*M_PI;
    double pitch = acos(Vg.dot(Eigen::Vector3d(0,0,1)))-0.5*M_PI;
    double pixelFor10Pitch = 5.0;
    double pitchOffsetAngle = -asin(pitch/M_PI*180.0/10.0*pixelFor10Pitch/30.0);
    cv::Point2f rollVector1 = 30*cv::Point2f(cos(roll),sin(roll));
    cv::Point2f rollVector2 = cv::Point2f(25,0);
    cv::Point2f rollVector3 = cv::Point2f(10,0);
    std::vector<cv::Point> pts;
    cv::ellipse2Poly(rollCenter,cv::Size(30,30),0,(roll-pitchOffsetAngle)/M_PI*180,(roll+pitchOffsetAngle)/M_PI*180+180,1,pts);
    cv::Point *points;
    points = &pts[0];
    int nbtab = pts.size();
    cv::fillPoly(filterState.img_,(const cv::Point**)&points,&nbtab,1,rollColor3);
    cv::line(filterState.img_,rollCenter+rollVector2,rollCenter+rollVector3,rollColor1, 2);
    cv::line(filterState.img_,rollCenter-rollVector2,rollCenter-rollVector3,rollColor1, 2);
    cv::ellipse(filterState.img_,rollCenter,cv::Size(10,10),0,0,180,rollColor1,2,8,0);
    cv::circle(filterState.img_,rollCenter,2,rollColor1,-1,8,0);
  }
};

}


#endif /* ROVIO_IMGUPDATE_HPP_ */