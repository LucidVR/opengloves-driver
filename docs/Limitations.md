
# Limitations

<br>

#### Missing Custom Controller Support

Many VR titles do not support finger tracking from **custom** <br>
controllers, requiring the need to emulate controller types.


We emulate the index controller to achieve this <br>
compatibility, which means that we are limited <br>
to the inputs that the index controller exposes. <br>

It is possible to emulate an index controller while <br>
providing your own input profiles and bindings, <br>
but we have chosen not to include that by default <br>
in the driver, as to preserve compatibility with <br>
default index controller bindings.

**➔**  *Implementing a custom controller can be achieved by* <br>
     *creating a derivative class of [`DeviceDriver`] in which* <br>
     *you specify the properties & settings of your controller.*

**➔**  An example of a fully custom controller is **[LucidGloveDriver]** <br>
     *which you are free to adapt to your needs.*

<br>

#### Dynamic Inputs

Due to how **OpenVR** wworks, inputs cannot be set ***dynamically***.

Our inputs for `Index Controller Emulated Devices` are ***fixed***.

**➔**  *However, you can define your own inputs in a* <br>
     *custom device with a different input profile.*


<!----------------------------------------------------------------------------->

[`DeviceDriver`]: ../src/DeviceDriver/DeviceDriver.cpp

[LucidGloveDriver]: https://github.com/LucidVR/opengloves-driver/blob/develop/src/DeviceDriver/LucidGloveDriver.cpp
