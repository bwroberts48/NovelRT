// Copyright © Matt Jones and Contributors. Licensed under the MIT Licence (MIT). See LICENCE.md in the repository root for more information.

#include <NovelRT.h>

//TODO: What do I do with these?
extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

static int average(lua_State* luaState) {
  int n = lua_gettop(luaState);
  double sum = 0;
  for (int i = 1; i <= n; i++)
  {
    sum += lua_tonumber(luaState, i);
  }
  lua_pushnumber(luaState, sum / n);
  lua_pushnumber(luaState, sum);
  return 2;
}

//TODO: Do we really need this anymore? No real reason to run this in WSL these days.
#ifdef WIN32
#define setenv(name, value, overwrite) \
  static_assert(overwrite != 0);       \
  _putenv_s(name, value)
#endif

int main(int /*argc*/, char* /*argv*/[])
{
  lua_State* L;

  std::unique_ptr<NovelRT::Graphics::ImageRect> novelChanRect;
  std::unique_ptr<NovelRT::Graphics::TextRect> textRect;
  std::unique_ptr<NovelRT::Graphics::BasicFillRect> lineRect;
  std::unique_ptr<NovelRT::Graphics::BasicFillRect> myBasicFillRect;
  std::unique_ptr<NovelRT::Graphics::BasicFillRect> playAudioButton;
  std::unique_ptr<NovelRT::Graphics::BasicFillRect> playAudioButtonTwoElectricBoogaloo;
  std::unique_ptr<NovelRT::Graphics::TextRect> playAudioText;
  std::unique_ptr<NovelRT::Input::BasicInteractionRect> interactionRect;
  std::unique_ptr<NovelRT::Input::BasicInteractionRect> memeInteractionRect;

#ifdef TEST_ANIM
  std::unique_ptr<NovelRT::Animation::SpriteAnimator> testAnim;
  std::unique_ptr<NovelRT::Graphics::ImageRect> animRect;

  bool shouldBeInIdle = true;
#endif

  std::filesystem::path executableDirPath = NovelRT::Utilities::Misc::getExecutableDirPath();
  std::filesystem::path resourcesDirPath = executableDirPath / "Resources";

  std::filesystem::path fontsDirPath = resourcesDirPath / "Fonts";
  std::filesystem::path imagesDirPath = resourcesDirPath / "Images";
  std::filesystem::path scriptsDirPath = resourcesDirPath / "Scripts";
  std::filesystem::path soundsDirPath = resourcesDirPath / "Sounds";

  //setenv("DISPLAY", "localhost:0", true);
  L = luaL_newstate();
  luaL_openlibs(L);
  lua_register(L, "average", average);
  luaL_dofile(L, (scriptsDirPath / "avg.lua").string().c_str());
  lua_close(L);

  auto runner = NovelRT::NovelRunner(0, "NovelRTTest");
  auto console = NovelRT::LoggingService(NovelRT::Utilities::Misc::CONSOLE_LOG_APP);
  auto audio = runner.getAudioService();
  audio.lock()->initializeAudio();
  auto bgm = audio.lock()->loadMusic((soundsDirPath / "marisa.ogg").string());
  auto jojo = audio.lock()->loadSound((soundsDirPath / "caution.wav").string());

#ifdef TEST_ANIM
  auto movingState = std::make_shared<NovelRT::Animation::SpriteAnimatorState>();
  auto idleState = std::make_shared<NovelRT::Animation::SpriteAnimatorState>();
  idleState->shouldLoop() = true;
  auto idleFrames = std::vector<NovelRT::Animation::SpriteAnimatorFrame>();

  for (int32_t i = 0; i < 10; i++) {
    auto frame = NovelRT::Animation::SpriteAnimatorFrame();
    frame.duration() = NovelRT::Timing::Timestamp::fromSeconds(0.1f);
    frame.texture() = runner.getRenderer().lock()->getTexture((imagesDirPath / "idle" / ("0-" + std::to_string(i) + ".png")).string());
    idleFrames.push_back(frame);
  }

  idleFrames.back().FrameExit += [&shouldBeInIdle] { shouldBeInIdle = false; };

  idleState->insertNewState(movingState, std::vector<std::function<bool()>> {[&shouldBeInIdle] { return !shouldBeInIdle;  }});

  idleState->frames() = idleFrames;


  movingState->shouldLoop() = true;
  movingState->insertNewState(idleState, std::vector<std::function<bool()>> {[&shouldBeInIdle] {return shouldBeInIdle;  }});

  auto movingFrames = std::vector<NovelRT::Animation::SpriteAnimatorFrame>();

  for (int32_t i = 0; i < 5; i++) {
    auto frame = NovelRT::Animation::SpriteAnimatorFrame();
    frame.duration() = NovelRT::Timing::Timestamp::fromSeconds(0.1f);
    frame.texture() = runner.getRenderer().lock()->getTexture((imagesDirPath / "right" / ("100-" + std::to_string(i) + ".png")).string());
    movingFrames.push_back(frame);
  }

  movingFrames.back().FrameExit += [&shouldBeInIdle] { shouldBeInIdle = true; };

  movingState->frames() = movingFrames;



  auto animTransform = NovelRT::Transform(NovelRT::Maths::GeoVector2<float>(1500, 900), 2, NovelRT::Maths::GeoVector2<float>(95 * 2, 98 * 2));
  animRect = runner.getRenderer().lock()->createImageRect(animTransform, 3, NovelRT::Graphics::RGBAConfig(255, 255, 255, 255));
  testAnim = std::make_unique<NovelRT::Animation::SpriteAnimator>(&runner, animRect.get());
  testAnim->insertNewState(idleState);

#endif

  auto novelChanTransform = NovelRT::Transform(NovelRT::Maths::GeoVector2<float>(1920 / 2, 1080 / 2), 2, NovelRT::Maths::GeoVector2<float>(456, 618));

  novelChanRect = runner.getRenderer().lock()->createImageRect(novelChanTransform, 3, (imagesDirPath / "novel-chan.png").string(), NovelRT::Graphics::RGBAConfig(255, 0, 255, 255));

  auto rubyGnomerTextTransform = NovelRT::Transform(NovelRT::Maths::GeoVector2<float>(novelChanTransform.position().getX() + 400, novelChanTransform.position().getY()), 0, NovelRT::Maths::GeoVector2<float>(1.0f, 1.0f));

  textRect = runner.getRenderer().lock()->createTextRect(rubyGnomerTextTransform, 2, NovelRT::Graphics::RGBAConfig(0, 255, 0, 255), 70, (fontsDirPath / "Gayathri-Regular.ttf").string());
  textRect->setText("RubyGnomer");

  auto lineTransform = NovelRT::Transform(rubyGnomerTextTransform.position(), 0, NovelRT::Maths::GeoVector2<float>(1000.0f, 2.0f));

  lineRect = runner.getRenderer().lock()->createBasicFillRect(lineTransform, 1, NovelRT::Graphics::RGBAConfig(255, 0, 0, 255));


  auto myTransform = NovelRT::Transform(NovelRT::Maths::GeoVector2<float>(1, 1), 0.0f, NovelRT::Maths::GeoVector2<float>(200, 300));
  myBasicFillRect = runner.getRenderer().lock()->createBasicFillRect(myTransform, 1, NovelRT::Graphics::RGBAConfig(255, 0, 0, 255));

  auto playButtonTransform = NovelRT::Transform(NovelRT::Maths::GeoVector2<float>(novelChanTransform.position().getX() - 500, novelChanTransform.position().getY()), 0, NovelRT::Maths::GeoVector2<float>(200, 200));
  playAudioButton = runner.getRenderer().lock()->createBasicFillRect(playButtonTransform, 3, NovelRT::Graphics::RGBAConfig(255, 0, 0, 70));
  auto playAudioTextTransform = playButtonTransform;
  playAudioTextTransform.scale() = NovelRT::Maths::GeoVector2<float>(1.0f, 1.0f);
  auto vec = playButtonTransform.position();
  vec.setX(playButtonTransform.position().getX() - 75);
  playAudioTextTransform.position() = vec;
  playAudioText = runner.getRenderer().lock()->createTextRect(playAudioTextTransform, 1, NovelRT::Graphics::RGBAConfig(0, 0, 0, 255), 36, (fontsDirPath / "Gayathri-Regular.ttf").string());
  playAudioText->setText("Play Audio");

  auto theRealMvpTransform = playButtonTransform;
  auto whatever = playButtonTransform.position();
  whatever.setX(whatever.getX() + 50);
  theRealMvpTransform.position() = whatever;

  memeInteractionRect = runner.getInteractionService().lock()->createBasicInteractionRect(theRealMvpTransform, -1);

  playAudioButtonTwoElectricBoogaloo = runner.getRenderer().lock()->createBasicFillRect(theRealMvpTransform, 2, NovelRT::Graphics::RGBAConfig(0, 255, 0, 70));


  runner.getDebugService().lock()->setIsFpsCounterVisible(true);

  runner.Update += [&](NovelRT::Timing::Timestamp delta) {
    const float rotationAmount = 45.0f;

    auto rotation = novelChanRect->transform().rotation();
    rotation += rotationAmount * delta.getSecondsFloat();
    novelChanRect->transform().rotation() = rotation;

    if (rotation > 360.0f)
    {
      rotation -= 360.0f;
    }

    novelChanRect->transform().rotation() = rotation;

    if (runner.getInteractionService().lock()->getKeyState(NovelRT::Input::KeyCode::W) == NovelRT::Input::KeyState::KeyDown) {
      console.logInfoLine("W is not idle!");
    }
  };

  interactionRect = runner.getInteractionService().lock()->createBasicInteractionRect(playButtonTransform, 2);
  memeInteractionRect->subscribedKey() = NovelRT::Input::KeyCode::LeftMouseButton;
  interactionRect->subscribedKey() = NovelRT::Input::KeyCode::RightMouseButton;

  auto loggingLevel = NovelRT::LogLevel::Debug;

  memeInteractionRect->Interacted += [&] {
    console.logDebug("WAHEYYY");
    audio.lock()->playSound(jojo, 0);
  };

  interactionRect->Interacted += [&] {
    console.log("Test button!", loggingLevel);


#ifdef TEST_ANIM
    switch (testAnim->getCurrentPlayState()) {
    case NovelRT::Animation::AnimatorPlayState::Playing:
      testAnim->stop();
      break;
    case NovelRT::Animation::AnimatorPlayState::Stopped:
      testAnim->play();
      break;
  }
#endif
};

  runner.SceneConstructionRequested += [&] {


    playAudioButton->executeObjectBehaviour();

    playAudioButtonTwoElectricBoogaloo->executeObjectBehaviour();

    playAudioText->executeObjectBehaviour();


    memeInteractionRect->executeObjectBehaviour();


    interactionRect->executeObjectBehaviour();

    novelChanRect->executeObjectBehaviour();

#ifdef TEST_ANIM
    animRect->executeObjectBehaviour();
#endif

    textRect->executeObjectBehaviour();
    myBasicFillRect->executeObjectBehaviour();
    lineRect->executeObjectBehaviour();
  };

  runner.getDotNetRuntimeService().lock()->initialize();
  audio.lock()->playMusic(bgm, -1);

  runner.runNovel();

  return 0;
}
