
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

##### Custom Implementation

If you want to implement your own device and use the utilities <br>
that **OpenGloves** provides, such as `Bone Calculations` and <br>
`Communication`, you will have to implement a custom driver.

To do this, you have to create your own class derived from [`DeviceDriver`], that <br> implement `StartingDevice`, `SetupProps`, `HandleInput` &  `StoppingDevice`.

<br>

#### Dynamic Inputs

Due to how **OpenVR** works, inputs cannot be set ***dynamically***.

Our inputs for `Index Controller Emulated Devices` are ***fixed*** <br>
to that of the index controller, and cannot have custom inputs.

**➔**  *However, you can define your own inputs in a* <br>
     *custom device with a different input profile.*


<!----------------------------------------------------------------------------->

[`DeviceDriver`]: ../src/DeviceDriver/DeviceDriver.cpp
