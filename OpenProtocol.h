
#pragma once
namespace OpenProtocol {
struct VirtualDevice {
    int type;
    int subType;
    int interfaces;
};

struct VirtualDeviceCameraOpen {
    int type;
    int subType;
    int interfaces;
    int width;
    int height;
    int ro;
    int facing;
};

struct VirtualDeviceGyroscope {
    float x;
    float y;
    float z;
};

enum class HardwareType {
    Camera,
    Sensor,
    Microphone
};

enum class CameraType {
    Camera
};

enum class SensorType {
    GPS,
    Accelerometer,
    Temperature,
    Gravity,
    Gyroscope,
    Light,
    LinearAcceleration,
    MagneticField,
    Orientation,
    Pressure,
    Proximity,
    RelativeHumidity,
    RotationVector
};

enum class MicrophoneType {
    Microphone
};

enum class GPSFunc {
    InjectLL,
    InjectErr
};

enum class GyroscopeFunc {
    InjectGyro,
    InjectErr,
};

enum class AccelerometerFunc {
    InjectAcc,
    InjectErr,
};

enum class OrientationFunc {
    InjectOrientation,

    InjectErr
};

enum class PressureFunc {
    InjectPressure,

    InjectErr
};

enum class ProximityFunc {
    InjectProximity,

    InjectErr,
};

enum class MicFunc {
    InjectAudio,

    OpenMic,

    CloseMic,

    InjectErr,
};

enum class CameraFunc {

    RegistCamera,

    GetCameraInfo,

    Open,

    Preview,

    Capture,

    Close,

    InjectErr,
};

enum class SensorFunc {

    Open  = 2,
    Close = 3,

};

}
