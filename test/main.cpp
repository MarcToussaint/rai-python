#include <Kin/kin.h>
#include <Kin/frame.h>
#include <Gui/opengl.h>
#include <KOMO/komo.h>
#include <Kin/TM_ContactConstraints.h>
//#include <KOMOcsail/komo-CSAIL.h>
#include <Kin/TM_default.h>

#include <ry/configuration.h>


//===========================================================================

void test(){

  auto K = ry::Configuration();
  auto D = K.camera();
  D.update("empty configuration\n -- hit ENTER here to continue", true);

  K.addFile("../rai-robotModels/pr2/pr2.g");
  K.addFile("kitchen.g");
  cout <<"joint names: " << I_conv(K.getJointNames()) <<endl;
  cout <<"frame names: " << I_conv(K.getFrameNames()) <<endl;
  D.update(true);

  //  K.editorFile("../rai-robotModels/baxter/baxter.g");

  //    q = K.getJointState()
  //    print('joint state: ', q)
  //    q[2] = q[2] + 1.
  //    K.setJointState(q)
  //    D.update(True)

  //    X = K.getFrameState()
  //    print('frame state: ', X)
  //    X = X + .1
  //    K.setFrameState(X)
  //    D.update(True)

  //    q = K.getJointState()
  //    print('joint state: ', q)
  //    q[2] = q[2] + 1.
  //    K.setJointState(q)
  //    D.update(True)

//  K.addFrame("camera", "head_tilt_link", "Q:<d(-90 1 0 0) d(180 0 0 1)> focalLength:.5");
//  auto C = K.camera("camera");

  K.addFrame("ball", "", "shape:sphere size:[0 0 0 .1] color:[1 1 0] X:<t(.8 .8 1.5)>" );
  D.update(true);

  K.addFrame("hand", "pr2L", "shape:ssBox size:[.3 .2 .1 .01] color:[1 1 0] Q:<t(0 0 0)>" );
  D.update(true);

  K.getPairDistance("hand", "ball");
  D.update(true);

  K.stash();
  {
    auto komo = K.komo_IK();
    komo.addObjective({}, {}, "eq", "posDiff", {"pr2L", "ball"});
//    komo.addObjectives2( { "feature:[eq posDiff pr2L ball]" } );
    komo.optimize();
    D.update(true);

    komo.clearObjectives();
    komo.addObjective({}, {}, "eq", "posDiff", {"pr2L", "ball"}, {}, {.1,.1,.1});
    komo.optimize();
  }
  D.update(true);

  K.pop();
  {
    auto komo = K.komo_path(1.);
//    komo.addObjectives2( { "time:[1.], feature:[eq posDiff pr2L ball]",
//                          "time:[1.], feature:[eq qRobot], order:1",
//                        } );
    komo.addObjectives({   I_feature({1.}, {"eq", "posDiff", "pr2L", "ball"}, I_args() ),
                           I_feature({1.}, {"eq", "qRobot"}, {{std::string("order"), {1.}}} )
                       });
    komo.optimize();
  }
  D.update(true);

}

//===========================================================================

void test_pickAndPlace(){
  auto K = ry::Configuration();
  auto D = K.camera();

  K.addFile("../rai-robotModels/pr2/pr2.g");
  K.addFile("kitchen.g");

  K.addFrame("item1", "sink1", "type:ssBox Q:<t(-.1 -.1 .52)> size:[.1 .1 .25 .02] color:[1. 0. 0.], contact" );
  K.addFrame("item2", "sink1", "type:ssBox Q:<t(.1 .1 .52)> size:[.1 .1 .25 .02] color:[1. 1. 0.], contact" );
  K.addFrame("tray", "stove1", "type:ssBox Q:<t(.0 .0 .42)> size:[.2 .2 .05 .02] color:[0. 1. 0.], contact" );

  auto obj1 = "item2";
  auto obj2 = "item1";
  auto tray = "tray";
  auto arm = "pr2L";
  auto table = "_13";

  uint T=6;
  auto komo = K.komo_CGO(T);

  komo.setCollionPairs({{obj1, obj2}});
  komo.addObjective({}, {}, "eq", "coll");
  komo.addObjective({}, {}, "ineq", "limits");

  komo.add_StableRelativePose({0, 1}, arm, obj1);
  komo.add_StableRelativePose({2, 3}, arm, obj2);
  komo.add_StableRelativePose({4, 5}, arm, tray);

  komo.add_StableRelativePose({1,2,3,4,5}, tray, obj1);
  komo.add_StableRelativePose({3,4,5}, tray, obj2);

  komo.add_StablePose({-1,0}, obj1);
  komo.add_StablePose({-1,0,1,2}, obj2);
  komo.add_StablePose({-1,0,1,2,3,4}, tray);

  komo.add_grasp(0, arm, obj1);
  komo.add_place(1, obj1, tray);

  komo.add_grasp(2, arm, obj2);
  komo.add_place(3, obj2, tray);

  komo.add_grasp(4, arm, tray);
  komo.add_place(5, tray, "_12");

  komo.optimize();

  for(int t=-1;t<T;t++){
    komo.getConfiguration(t);
    D.update(true);
  }
}


//===========================================================================

void test_lgp(){
  auto K = ry::Configuration();
  auto D = K.camera();

  K.addFile("lgp-example.g");
  D.update(true);

  auto lgp = K.lgp();

  lgp.optimizeFixedSequence("(grasp baxterR stick) (push stickTip redBall table1) (grasp baxterL redBall) ");
}

//===========================================================================

int main(int argc,char** argv){
  rai::initCmdLine(argc,argv);

//  test();
  test_pickAndPlace();
//  test_lgp();

  return 0;
}


