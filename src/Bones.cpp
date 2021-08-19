#include <Bones.h>
#include <DriverLog.h>

enum class FingerIndex : int {
   Thumb,
   Index,
   Middle,
   Ring,
   Pinky,
   None = -1
};

// these poses come from Valve's Index Controllers so share the same root bone to wrist
// geometry assumptions.
float animationFrameTimes[NUM_ANIMATION_FRAMES] = {0.0f, 1.0f};
vr::VRBoneTransform_t rightAnimationFrames[NUM_ANIMATION_FRAMES][NUM_BONES] = {
   // Open
   {
      {{0.000000f, 0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
      {{0.034038f, 0.036503f, 0.164722f, 1.000000f}, {-0.055147f, -0.078608f, 0.920279f, -0.379296f}},
      {{0.012083f, 0.028070f, 0.025050f, 1.000000f}, {0.567418f, -0.464112f, 0.623374f, -0.272106f}},
      {{-0.040406f, -0.000000f, 0.000000f, 1.000000f}, {0.994838f, 0.082939f, 0.019454f, 0.055130f}},
      {{-0.032517f, -0.000000f, -0.000000f, 1.000000f}, {0.974793f, -0.003213f, 0.021867f, -0.222015f}},
      {{-0.030464f, 0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
      {{-0.000632f, 0.026866f, 0.015002f, 1.000000f}, {0.421979f, -0.644251f, 0.422133f, 0.478202f}},
      {{-0.074204f, 0.005002f, -0.000234f, 1.000000f}, {0.995332f, 0.007007f, -0.039124f, 0.087949f}},
      {{-0.043930f, 0.000000f, 0.000000f, 1.000000f}, {0.997891f, 0.045808f, 0.002142f, -0.045943f}},
      {{-0.028695f, -0.000000f, -0.000000f, 1.000000f}, {0.999649f, 0.001850f, -0.022782f, -0.013409f}},
      {{-0.022821f, -0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, 0.000000f, -0.000000f}},
      {{-0.002177f, 0.007120f, 0.016319f, 1.000000f}, {0.541276f, -0.546723f, 0.460749f, 0.442520f}},
      {{-0.070953f, -0.000779f, -0.000997f, 1.000000f}, {0.980294f, -0.167261f, -0.078959f, 0.069368f}},
      {{-0.043108f, -0.000000f, -0.000000f, 1.000000f}, {0.997947f, 0.018493f, 0.013192f, 0.059886f}},
      {{-0.033266f, -0.000000f, -0.000000f, 1.000000f}, {0.997394f, -0.003328f, -0.028225f, -0.066315f}},
      {{-0.025892f, 0.000000f, -0.000000f, 1.000000f}, {0.999195f, -0.000000f, 0.000000f, 0.040126f}},
      {{-0.000513f, -0.006545f, 0.016348f, 1.000000f}, {0.550143f, -0.516692f, 0.429888f, 0.495548f}},
      {{-0.065876f, -0.001786f, -0.000693f, 1.000000f}, {0.990420f, -0.058696f, -0.101820f, 0.072495f}},
      {{-0.040697f, -0.000000f, -0.000000f, 1.000000f}, {0.999545f, -0.002240f, 0.000004f, 0.030081f}},
      {{-0.028747f, 0.000000f, 0.000000f, 1.000000f}, {0.999102f, -0.000721f, -0.012693f, 0.040420f}},
      {{-0.022430f, 0.000000f, -0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
      {{0.002478f, -0.018981f, 0.015214f, 1.000000f}, {0.523940f, -0.526918f, 0.326740f, 0.584025f}},
      {{-0.062878f, -0.002844f, -0.000332f, 1.000000f}, {0.986609f, -0.059615f, -0.135163f, 0.069132f}},
      {{-0.030220f, -0.000000f, -0.000000f, 1.000000f}, {0.994317f, 0.001896f, -0.000132f, 0.106446f}},
      {{-0.018187f, -0.000000f, -0.000000f, 1.000000f}, {0.995931f, -0.002010f, -0.052079f, -0.073526f}},
      {{-0.018018f, -0.000000f, 0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
      {{0.006059f, 0.056285f, 0.060064f, 1.000000f}, {0.737238f, 0.202745f, -0.594267f, -0.249441f}},
      {{0.040416f, -0.043018f, 0.019345f, 1.000000f}, {-0.290331f, 0.623527f, 0.663809f, 0.293734f}},
      {{0.039354f, -0.075674f, 0.047048f, 1.000000f}, {-0.187047f, 0.678062f, 0.659285f, 0.265683f}},
      {{0.038340f, -0.090987f, 0.082579f, 1.000000f}, {-0.183037f, 0.736793f, 0.634757f, 0.143936f}},
      {{0.031806f, -0.087214f, 0.121015f, 1.000000f}, {-0.003659f, 0.758407f, 0.639342f, 0.126678f}},
   },
   // Closed
   {
      {{0.000000f, 0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
      {{0.034038f, 0.036503f, 0.164722f, 1.000000f}, {-0.055147f, -0.078608f, 0.920279f, -0.379296f}},
      {{0.016305f, 0.027529f, 0.017800f, 1.000000f}, {0.483332f, -0.225703f, 0.836342f, -0.126413f}},
      {{-0.040406f, -0.000000f, 0.000000f, 1.000000f}, {0.894335f, -0.013302f, -0.082902f, 0.439448f}},
      {{-0.032517f, -0.000000f, -0.000000f, 1.000000f}, {0.842428f, 0.000655f, 0.001244f, 0.538807f}},
      {{-0.030464f, 0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
      {{-0.003802f, 0.021514f, 0.012803f, 1.000000f}, {0.395174f, -0.617314f, 0.449185f, 0.510874f}},
      {{-0.074204f, 0.005002f, -0.000234f, 1.000000f}, {0.737291f, -0.032006f, -0.115013f, 0.664944f}},
      {{-0.043287f, 0.000000f, 0.000000f, 1.000000f}, {0.611381f, 0.003287f, 0.003823f, 0.791321f}},
      {{-0.028275f, -0.000000f, -0.000000f, 1.000000f}, {0.745388f, -0.000684f, -0.000945f, 0.666629f}},
      {{-0.022821f, -0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, 0.000000f, -0.000000f}},
      {{-0.005787f, 0.006806f, 0.016534f, 1.000000f}, {0.522315f, -0.514203f, 0.483700f, 0.478348f}},
      {{-0.070953f, -0.000779f, -0.000997f, 1.000000f}, {0.723653f, -0.097901f, 0.048546f, 0.681458f}},
      {{-0.043108f, -0.000000f, -0.000000f, 1.000000f}, {0.637464f, -0.002366f, -0.002831f, 0.770472f}},
      {{-0.033266f, -0.000000f, -0.000000f, 1.000000f}, {0.658008f, 0.002610f, 0.003196f, 0.753000f}},
      {{-0.025892f, 0.000000f, -0.000000f, 1.000000f}, {0.999195f, -0.000000f, 0.000000f, 0.040126f}},
      {{-0.004123f, -0.006858f, 0.016563f, 1.000000f}, {0.523374f, -0.489609f, 0.463997f, 0.520644f}},
      {{-0.065876f, -0.001786f, -0.000693f, 1.000000f}, {0.759970f, -0.055609f, 0.011571f, 0.647471f}},
      {{-0.040331f, -0.000000f, -0.000000f, 1.000000f}, {0.664315f, 0.001595f, 0.001967f, 0.747449f}},
      {{-0.028489f, 0.000000f, 0.000000f, 1.000000f}, {0.626957f, -0.002784f, -0.003234f, 0.779042f}},
      {{-0.022430f, 0.000000f, -0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
      {{-0.001131f, -0.019295f, 0.015429f, 1.000000f}, {0.477833f, -0.479766f, 0.379935f, 0.630198f}},
      {{-0.062878f, -0.002844f, -0.000332f, 1.000000f}, {0.827001f, 0.034282f, 0.003440f, 0.561144f}},
      {{-0.029874f, -0.000000f, -0.000000f, 1.000000f}, {0.702185f, -0.006716f, -0.009289f, 0.711903f}},
      {{-0.017979f, -0.000000f, -0.000000f, 1.000000f}, {0.676853f, 0.007956f, 0.009917f, 0.736009f}},
      {{-0.018018f, -0.000000f, 0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
      {{-0.019716f, 0.002802f, 0.093937f, 1.000000f}, {0.377286f, -0.540831f, -0.150446f, 0.736562f}},
      {{-0.000171f, 0.016473f, 0.096515f, 1.000000f}, {-0.006456f, 0.022747f, 0.932927f, 0.359287f}},
      {{-0.000448f, 0.001536f, 0.116543f, 1.000000f}, {-0.039357f, 0.105143f, 0.928833f, 0.353079f}},
      {{-0.003949f, -0.014869f, 0.130608f, 1.000000f}, {-0.055071f, 0.068695f, 0.944016f, 0.317933f}},
      {{-0.003263f, -0.034685f, 0.139926f, 1.000000f}, {0.019690f, -0.100741f, 0.957331f, 0.270149f}},
   },
};
vr::VRBoneTransform_t leftAnimationFrames[NUM_ANIMATION_FRAMES][NUM_BONES] = {
   // Open
   {
      {{0.000000f, 0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
      {{-0.034038f, 0.036503f, 0.164722f, 1.000000f}, {-0.055147f, -0.078608f, -0.920279f, 0.379296f}},
      {{-0.012083f, 0.028070f, 0.025050f, 1.000000f}, {0.464112f, 0.567418f, 0.272106f, 0.623374f}},
      {{0.040406f, 0.000000f, -0.000000f, 1.000000f}, {0.994838f, 0.082939f, 0.019454f, 0.055130f}},
      {{0.032517f, 0.000000f, 0.000000f, 1.000000f}, {0.974793f, -0.003213f, 0.021867f, -0.222015f}},
      {{0.030464f, -0.000000f, -0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
      {{0.000632f, 0.026866f, 0.015002f, 1.000000f}, {0.644251f, 0.421979f, -0.478202f, 0.422133f}},
      {{0.074204f, -0.005002f, 0.000234f, 1.000000f}, {0.995332f, 0.007007f, -0.039124f, 0.087949f}},
      {{0.043930f, -0.000000f, -0.000000f, 1.000000f}, {0.997891f, 0.045808f, 0.002142f, -0.045943f}},
      {{0.028695f, 0.000000f, 0.000000f, 1.000000f}, {0.999649f, 0.001850f, -0.022782f, -0.013409f}},
      {{0.022821f, 0.000000f, -0.000000f, 1.000000f}, {1.000000f, -0.000000f, 0.000000f, -0.000000f}},
      {{0.002177f, 0.007120f, 0.016319f, 1.000000f}, {0.546723f, 0.541276f, -0.442520f, 0.460749f}},
      {{0.070953f, 0.000779f, 0.000997f, 1.000000f}, {0.980294f, -0.167261f, -0.078959f, 0.069368f}},
      {{0.043108f, 0.000000f, 0.000000f, 1.000000f}, {0.997947f, 0.018493f, 0.013192f, 0.059886f}},
      {{0.033266f, 0.000000f, 0.000000f, 1.000000f}, {0.997394f, -0.003328f, -0.028225f, -0.066315f}},
      {{0.025892f, -0.000000f, 0.000000f, 1.000000f}, {0.999195f, -0.000000f, 0.000000f, 0.040126f}},
      {{0.000513f, -0.006545f, 0.016348f, 1.000000f}, {0.516692f, 0.550143f, -0.495548f, 0.429888f}},
      {{0.065876f, 0.001786f, 0.000693f, 1.000000f}, {0.990420f, -0.058696f, -0.101820f, 0.072495f}},
      {{0.040697f, 0.000000f, 0.000000f, 1.000000f}, {0.999545f, -0.002240f, 0.000004f, 0.030081f}},
      {{0.028747f, -0.000000f, -0.000000f, 1.000000f}, {0.999102f, -0.000721f, -0.012693f, 0.040420f}},
      {{0.022430f, -0.000000f, 0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
      {{-0.002478f, -0.018981f, 0.015214f, 1.000000f}, {0.526918f, 0.523940f, -0.584025f, 0.326740f}},
      {{0.062878f, 0.002844f, 0.000332f, 1.000000f}, {0.986609f, -0.059615f, -0.135163f, 0.069132f}},
      {{0.030220f, 0.000000f, 0.000000f, 1.000000f}, {0.994317f, 0.001896f, -0.000132f, 0.106446f}},
      {{0.018187f, 0.000000f, 0.000000f, 1.000000f}, {0.995931f, -0.002010f, -0.052079f, -0.073526f}},
      {{0.018018f, 0.000000f, -0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
      {{-0.006059f, 0.056285f, 0.060064f, 1.000000f}, {0.737238f, 0.202745f, 0.594267f, 0.249441f}},
      {{-0.040416f, -0.043018f, 0.019345f, 1.000000f}, {-0.290331f, 0.623527f, -0.663809f, -0.293734f}},
      {{-0.039354f, -0.075674f, 0.047048f, 1.000000f}, {-0.187047f, 0.678062f, -0.659285f, -0.265683f}},
      {{-0.038340f, -0.090987f, 0.082579f, 1.000000f}, {-0.183037f, 0.736793f, -0.634757f, -0.143936f}},
      {{-0.031806f, -0.087214f, 0.121015f, 1.000000f}, {-0.003659f, 0.758407f, -0.639342f, -0.126678f}},
   },
   // Closed
   {
      {{0.000000f, 0.000000f, 0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
      {{-0.034038f, 0.036503f, 0.164722f, 1.000000f}, {-0.055147f, -0.078608f, -0.920279f, 0.379296f}},
      {{-0.016305f, 0.027529f, 0.017800f, 1.000000f}, {0.225703f, 0.483332f, 0.126413f, 0.836342f}},
      {{0.040406f, 0.000000f, -0.000000f, 1.000000f}, {0.894335f, -0.013302f, -0.082902f, 0.439448f}},
      {{0.032517f, 0.000000f, 0.000000f, 1.000000f}, {0.842428f, 0.000655f, 0.001244f, 0.538807f}},
      {{0.030464f, -0.000000f, -0.000000f, 1.000000f}, {1.000000f, -0.000000f, -0.000000f, 0.000000f}},
      {{0.003802f, 0.021514f, 0.012803f, 1.000000f}, {0.617314f, 0.395175f, -0.510874f, 0.449185f}},
      {{0.074204f, -0.005002f, 0.000234f, 1.000000f}, {0.737291f, -0.032006f, -0.115013f, 0.664944f}},
      {{0.043287f, -0.000000f, -0.000000f, 1.000000f}, {0.611381f, 0.003287f, 0.003823f, 0.791321f}},
      {{0.028275f, 0.000000f, 0.000000f, 1.000000f}, {0.745388f, -0.000684f, -0.000945f, 0.666629f}},
      {{0.022821f, 0.000000f, -0.000000f, 1.000000f}, {1.000000f, -0.000000f, 0.000000f, -0.000000f}},
      {{0.005787f, 0.006806f, 0.016534f, 1.000000f}, {0.514203f, 0.522315f, -0.478348f, 0.483700f}},
      {{0.070953f, 0.000779f, 0.000997f, 1.000000f}, {0.723653f, -0.097901f, 0.048546f, 0.681458f}},
      {{0.043108f, 0.000000f, 0.000000f, 1.000000f}, {0.637464f, -0.002366f, -0.002831f, 0.770472f}},
      {{0.033266f, 0.000000f, 0.000000f, 1.000000f}, {0.658008f, 0.002610f, 0.003196f, 0.753000f}},
      {{0.025892f, -0.000000f, 0.000000f, 1.000000f}, {0.999195f, -0.000000f, 0.000000f, 0.040126f}},
      {{0.004123f, -0.006858f, 0.016563f, 1.000000f}, {0.489609f, 0.523374f, -0.520644f, 0.463997f}},
      {{0.065876f, 0.001786f, 0.000693f, 1.000000f}, {0.759970f, -0.055609f, 0.011571f, 0.647471f}},
      {{0.040331f, 0.000000f, 0.000000f, 1.000000f}, {0.664315f, 0.001595f, 0.001967f, 0.747449f}},
      {{0.028489f, -0.000000f, -0.000000f, 1.000000f}, {0.626957f, -0.002784f, -0.003234f, 0.779042f}},
      {{0.022430f, -0.000000f, 0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
      {{0.001131f, -0.019295f, 0.015429f, 1.000000f}, {0.479766f, 0.477833f, -0.630198f, 0.379934f}},
      {{0.062878f, 0.002844f, 0.000332f, 1.000000f}, {0.827001f, 0.034282f, 0.003440f, 0.561144f}},
      {{0.029874f, 0.000000f, 0.000000f, 1.000000f}, {0.702185f, -0.006716f, -0.009289f, 0.711903f}},
      {{0.017979f, 0.000000f, 0.000000f, 1.000000f}, {0.676853f, 0.007956f, 0.009917f, 0.736009f}},
      {{0.018018f, 0.000000f, -0.000000f, 1.000000f}, {1.000000f, 0.000000f, 0.000000f, 0.000000f}},
      {{0.019716f, 0.002802f, 0.093937f, 1.000000f}, {0.377286f, -0.540831f, 0.150446f, -0.736562f}},
      {{0.000171f, 0.016473f, 0.096515f, 1.000000f}, {-0.006456f, 0.022747f, -0.932927f, -0.359287f}},
      {{0.000448f, 0.001536f, 0.116543f, 1.000000f}, {-0.039357f, 0.105143f, -0.928833f, -0.353079f}},
      {{0.003949f, -0.014869f, 0.130608f, 1.000000f}, {-0.055071f, 0.068695f, -0.944016f, -0.317933f}},
      {{0.003263f, -0.034685f, 0.139926f, 1.000000f}, {0.019690f, -0.100741f, -0.957331f, -0.270149f}},
   },
};

/**
 *Linear interpolation between a and b.
 **/
static float Lerp(const float a, const float b, const float f) { return a + f * (b - a); }

static vr::HmdQuaternionf_t CalculateOrientation(const float transform, const int boneIndex, const vr::VRBoneTransform_t* startPose, const vr::VRBoneTransform_t* endPose) {
  const vr::HmdQuaternionf_t startPoseOrientation = startPose[boneIndex].orientation;
  const vr::HmdQuaternionf_t endPoseOrientation = endPose[boneIndex].orientation;

  vr::HmdQuaternionf_t result{};
  result.w = Lerp(startPoseOrientation.w, endPoseOrientation.w, transform);
  result.x = Lerp(startPoseOrientation.x, endPoseOrientation.x, transform);
  result.y = Lerp(startPoseOrientation.y, endPoseOrientation.y, transform);
  result.z = Lerp(startPoseOrientation.z, endPoseOrientation.z, transform);

  return result;
}
static vr::HmdVector4_t CalculatePosition(const float transform, const int boneIndex, const vr::VRBoneTransform_t* startPose, const vr::VRBoneTransform_t* endPose) {
  const vr::HmdVector4_t startPosePosition = startPose[boneIndex].position;
  const vr::HmdVector4_t endPosePosition = endPose[boneIndex].position;

  vr::HmdVector4_t result{};
  result.v[0] = Lerp(startPosePosition.v[0], endPosePosition.v[0], transform);
  result.v[1] = Lerp(startPosePosition.v[1], endPosePosition.v[1], transform);
  result.v[2] = Lerp(startPosePosition.v[2], endPosePosition.v[2], transform);
  result.v[3] = Lerp(startPosePosition.v[3], endPosePosition.v[3], transform);

  return result;
}

// Transform should be between 0-1
static void ComputeBoneFlexion(vr::VRBoneTransform_t* boneTransform, float transform, int index, const vr::VRBoneTransform_t* startPose, const vr::VRBoneTransform_t* endPose) {
  boneTransform->orientation = CalculateOrientation(transform, index, startPose, endPose);
  boneTransform->position = CalculatePosition(transform, index, startPose, endPose);
}

static FingerIndex FingerFromBone(vr::BoneIndex_t bone) {
  switch (bone) {
    case eBone_Thumb0:
    case eBone_Thumb1:
    case eBone_Thumb2:
    case eBone_Thumb3:
    case eBone_Aux_Thumb:
      return FingerIndex::Thumb;
    case eBone_IndexFinger0:
    case eBone_IndexFinger1:
    case eBone_IndexFinger2:
    case eBone_IndexFinger3:
    case eBone_IndexFinger4:
    case eBone_Aux_IndexFinger:
      return FingerIndex::Index;
    case eBone_MiddleFinger0:
    case eBone_MiddleFinger1:
    case eBone_MiddleFinger2:
    case eBone_MiddleFinger3:
    case eBone_MiddleFinger4:
    case eBone_Aux_MiddleFinger:
      return FingerIndex::Middle;
    case eBone_RingFinger0:
    case eBone_RingFinger1:
    case eBone_RingFinger2:
    case eBone_RingFinger3:
    case eBone_RingFinger4:
    case eBone_Aux_RingFinger:
      return FingerIndex::Ring;
    case eBone_PinkyFinger0:
    case eBone_PinkyFinger1:
    case eBone_PinkyFinger2:
    case eBone_PinkyFinger3:
    case eBone_PinkyFinger4:
    case eBone_Aux_PinkyFinger:
      return FingerIndex::Pinky;

    default:
      return FingerIndex::None;
  }
}

void ComputeHand(vr::VRBoneTransform_t* skeleton, const std::array<float, 5>& flexion, const bool isRightHand) {
  const auto& animationFrames = isRightHand ? rightAnimationFrames : leftAnimationFrames;
  for (int i = 0; i < NUM_BONES; i++) {
    FingerIndex fingerNum = FingerFromBone(i);
    if (fingerNum != FingerIndex::None) {
      // Determine the animation frames for the specified finger
      float t = flexion[(int)fingerNum];
      const vr::VRBoneTransform_t* startPose;
      const vr::VRBoneTransform_t* endPose;
      if (t < animationFrameTimes[0]) {
        // Clamp at beginning of animation
        t = 0.0;
        startPose = animationFrames[0];
        endPose = animationFrames[1];
      } else if (t >= animationFrameTimes[NUM_ANIMATION_FRAMES - 1]) {
        // Clamp at end of animation
        t = 1.0;
        startPose = animationFrames[NUM_ANIMATION_FRAMES - 2];
        endPose = animationFrames[NUM_ANIMATION_FRAMES - 1];
      } else {
        // Find the current frame
        int currentFrame = 0;
        for (int j = 0; j < NUM_ANIMATION_FRAMES - 1; j++)
          if (animationFrameTimes[j] <= t) {
            currentFrame = j;
            break;
          }

        // Get the positions for current/next
        startPose = animationFrames[currentFrame];
        endPose = animationFrames[currentFrame + 1];

        // Get the new time for between frames
        const float startTime = animationFrameTimes[currentFrame];
        const float endTime = animationFrameTimes[currentFrame + 1];
        if (startTime < endTime)
          // Find the percentage through the current frame
          t = (t - startTime) / (endTime - startTime);
        else
          // Just show the start pos
          t = 0.0;
      }

      // Compute the transform from the given animation frames
      ComputeBoneFlexion(&skeleton[i], t, i, startPose, endPose);
    }
  }
}