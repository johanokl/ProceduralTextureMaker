<!DOCTYPE TextureSet>
<TextureSet>
   <Nodes>
      <Node id="1" name="Deep space">
         <pos x="-896.951" y="-1134.26"/>
         <generator name="Gradient"/>
         <Settings>
            <setting id="endcolor" type="QColor" value="#02000d"/>
            <setting id="endposx" type="double" value="18"/>
            <setting id="endposy" type="double" value="15"/>
            <setting id="gradient" type="QString" value="Radial Gradient"/>
            <setting id="middlecolor" type="QColor" value="#10052f"/>
            <setting id="middleposition" type="double" value="45"/>
            <setting id="radius" type="double" value="95"/>
            <setting id="spread" type="QString" value="Pad Spread"/>
            <setting id="startcolor" type="QColor" value="#32116b"/>
            <setting id="startposx" type="double" value="-20"/>
            <setting id="startposy" type="double" value="-25"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="2" name="Blue diamond">
         <pos x="-558.803" y="-763.044"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#7ee8ff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="4.2"/>
            <setting id="offsetleft" type="double" value="-38"/>
            <setting id="offsettop" type="double" value="-34"/>
            <setting id="rotation" type="double" value="45"/>
            <setting id="width" type="double" value="0.6"/>
         </Settings>
         <Sources>
            <source slot="0" source="1"/>
         </Sources>
      </Node>
      <Node id="3" name="White star">
         <pos x="37.0843" y="-614.273"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ffffff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="2.6"/>
            <setting id="offsetleft" type="double" value="-12"/>
            <setting id="offsettop" type="double" value="-22"/>
            <setting id="rotation" type="double" value="45"/>
            <setting id="width" type="double" value="0.4"/>
         </Settings>
         <Sources>
            <source slot="0" source="37"/>
         </Sources>
      </Node>
      <Node id="4" name="Pink star">
         <pos x="540.898" y="-539.168"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ff8cdd"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="3.2"/>
            <setting id="offsetleft" type="double" value="8"/>
            <setting id="offsettop" type="double" value="-40"/>
            <setting id="rotation" type="double" value="45"/>
            <setting id="width" type="double" value="0.5"/>
         </Settings>
         <Sources>
            <source slot="0" source="38"/>
         </Sources>
      </Node>
      <Node id="5" name="Gold star">
         <pos x="1097.17" y="-431.432"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ffe18a"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="3.8"/>
            <setting id="offsetleft" type="double" value="36"/>
            <setting id="offsettop" type="double" value="-12"/>
            <setting id="rotation" type="double" value="45"/>
            <setting id="width" type="double" value="0.6"/>
         </Settings>
         <Sources>
            <source slot="0" source="39"/>
         </Sources>
      </Node>
      <Node id="6" name="Planet base">
         <pos x="-911.845" y="-247.038"/>
         <generator name="Fill"/>
         <Settings>
            <setting id="color" type="QColor" value="#07163d"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="7" name="Planet bands">
         <pos x="-355.219" y="-27.7659"/>
         <generator name="Sine plasma"/>
         <Settings>
            <setting id="color" type="QColor" value="#ff3ca6"/>
            <setting id="xfrequency" type="double" value="3"/>
            <setting id="xoffset" type="double" value="-18"/>
            <setting id="yfrequency" type="double" value="30"/>
            <setting id="yoffset" type="double" value="12"/>
         </Settings>
         <Sources>
            <source slot="0" source="6"/>
         </Sources>
      </Node>
      <Node id="8" name="Tilt planet bands">
         <pos x="140.261" y="-102.037"/>
         <generator name="Transform"/>
         <Settings>
            <setting id="backgroundcolor" type="QColor" value="#00000000"/>
            <setting id="firstXtiles" type="int" value="1"/>
            <setting id="firstYtiles" type="int" value="1"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="0"/>
            <setting id="rotation" type="double" value="-16"/>
            <setting id="secondXtiles" type="int" value="1"/>
            <setting id="secondYtiles" type="int" value="1"/>
            <setting id="xscale" type="double" value="100"/>
            <setting id="yscale" type="double" value="100"/>
         </Settings>
         <Sources>
            <source slot="0" source="7"/>
         </Sources>
      </Node>
      <Node id="9" name="Planet mask">
         <pos x="203.985" y="318.303"/>
         <generator name="Circle"/>
         <Settings>
            <setting id="color" type="QColor" value="#ffffff"/>
            <setting id="innerradius" type="double" value="0"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="0"/>
            <setting id="outerradius" type="double" value="82"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="10" name="Masked planet">
         <pos x="676.347" y="16.3574"/>
         <generator name="Set channels"/>
         <Settings>
            <setting id="channelAlpha" type="QString" value="Second's alpha"/>
            <setting id="channelBlue" type="QString" value="First's blue"/>
            <setting id="channelGreen" type="QString" value="First's green"/>
            <setting id="channelRed" type="QString" value="First's red"/>
         </Settings>
         <Sources>
            <source slot="0" source="8"/>
            <source slot="1" source="9"/>
         </Sources>
      </Node>
      <Node id="11" name="Planet glow">
         <pos x="1172.61" y="33.1398"/>
         <generator name="Glow"/>
         <Settings>
            <setting id="color" type="QColor" value="#4de8ff"/>
            <setting id="cutoutx" type="double" value="95"/>
            <setting id="cutouty" type="double" value="95"/>
            <setting id="firstblurlevel" type="double" value="5"/>
            <setting id="includesource" type="bool" value="true"/>
            <setting id="mode" type="QString" value="Enlarge"/>
            <setting id="ontop" type="bool" value="false"/>
            <setting id="secondblurlevel" type="double" value="3"/>
            <setting id="size" type="double" value="5"/>
         </Settings>
         <Sources>
            <source slot="0" source="10"/>
         </Sources>
      </Node>
      <Node id="12" name="Place planet">
         <pos x="1601.59" y="134.984"/>
         <generator name="Transform"/>
         <Settings>
            <setting id="backgroundcolor" type="QColor" value="#00000000"/>
            <setting id="firstXtiles" type="int" value="1"/>
            <setting id="firstYtiles" type="int" value="1"/>
            <setting id="offsetleft" type="double" value="31"/>
            <setting id="offsettop" type="double" value="-28"/>
            <setting id="rotation" type="double" value="0"/>
            <setting id="secondXtiles" type="int" value="1"/>
            <setting id="secondYtiles" type="int" value="1"/>
            <setting id="xscale" type="double" value="38"/>
            <setting id="yscale" type="double" value="38"/>
         </Settings>
         <Sources>
            <source slot="0" source="11"/>
         </Sources>
      </Node>
      <Node id="13" name="Planet ring">
         <pos x="639.98" y="496.979"/>
         <generator name="Circle"/>
         <Settings>
            <setting id="color" type="QColor" value="#ffd45c"/>
            <setting id="innerradius" type="double" value="72"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="0"/>
            <setting id="outerradius" type="double" value="84"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="14" name="Shape and place ring">
         <pos x="1329.11" y="515.163"/>
         <generator name="Transform"/>
         <Settings>
            <setting id="backgroundcolor" type="QColor" value="#00000000"/>
            <setting id="firstXtiles" type="int" value="1"/>
            <setting id="firstYtiles" type="int" value="1"/>
            <setting id="offsetleft" type="double" value="31"/>
            <setting id="offsettop" type="double" value="-28"/>
            <setting id="rotation" type="double" value="-16"/>
            <setting id="secondXtiles" type="int" value="1"/>
            <setting id="secondYtiles" type="int" value="1"/>
            <setting id="xscale" type="double" value="58"/>
            <setting id="yscale" type="double" value="15"/>
         </Settings>
         <Sources>
            <source slot="0" source="13"/>
         </Sources>
      </Node>
      <Node id="15" name="Ringed planet">
         <pos x="2051.14" y="454.536"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" type="double" value="100"/>
            <setting id="mode" type="QString" value="Normal"/>
            <setting id="order" type="QString" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="14"/>
            <source slot="1" source="12"/>
         </Sources>
      </Node>
      <Node id="16" name="Space and planet">
         <pos x="2867.34" y="-336.064"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" type="double" value="100"/>
            <setting id="mode" type="QString" value="Normal"/>
            <setting id="order" type="QString" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="44"/>
            <source slot="1" source="15"/>
         </Sources>
      </Node>
      <Node id="17" name="Rocket nose">
         <pos x="-345.242" y="383.772"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ff3c73"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="16"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="-24"/>
            <setting id="rotation" type="double" value="45"/>
            <setting id="width" type="double" value="16"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="18" name="Rocket body">
         <pos x="-371.664" y="837.452"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#dff5ff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="44"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="0"/>
            <setting id="rotation" type="double" value="0"/>
            <setting id="width" type="double" value="20"/>
         </Settings>
         <Sources>
            <source slot="0" source="17"/>
         </Sources>
      </Node>
      <Node id="19" name="Left fin">
         <pos x="83.5099" y="745.062"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ff3c73"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="25"/>
            <setting id="offsetleft" type="double" value="-11"/>
            <setting id="offsettop" type="double" value="16"/>
            <setting id="rotation" type="double" value="25"/>
            <setting id="width" type="double" value="11"/>
         </Settings>
         <Sources>
            <source slot="0" source="18"/>
         </Sources>
      </Node>
      <Node id="20" name="Right fin">
         <pos x="-95.8646" y="1502.83"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ff3c73"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="25"/>
            <setting id="offsetleft" type="double" value="11"/>
            <setting id="offsettop" type="double" value="16"/>
            <setting id="rotation" type="double" value="155"/>
            <setting id="width" type="double" value="11"/>
         </Settings>
         <Sources>
            <source slot="0" source="19"/>
         </Sources>
      </Node>
      <Node id="21" name="Rocket stripe">
         <pos x="851.604" y="941.382"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ffb52e"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="6"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="9"/>
            <setting id="rotation" type="double" value="0"/>
            <setting id="width" type="double" value="21"/>
         </Settings>
         <Sources>
            <source slot="0" source="36"/>
         </Sources>
      </Node>
      <Node id="22" name="Window frame">
         <pos x="1319.09" y="934.745"/>
         <generator name="Circle"/>
         <Settings>
            <setting id="color" type="QColor" value="#15214d"/>
            <setting id="innerradius" type="double" value="0"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="-8"/>
            <setting id="outerradius" type="double" value="17"/>
         </Settings>
         <Sources>
            <source slot="0" source="21"/>
         </Sources>
      </Node>
      <Node id="23" name="Window glass">
         <pos x="1669.62" y="927.301"/>
         <generator name="Circle"/>
         <Settings>
            <setting id="color" type="QColor" value="#48e9ff"/>
            <setting id="innerradius" type="double" value="0"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="-8"/>
            <setting id="outerradius" type="double" value="11"/>
         </Settings>
         <Sources>
            <source slot="0" source="22"/>
         </Sources>
      </Node>
      <Node id="24" name="Rocket glow">
         <pos x="2118.19" y="863.749"/>
         <generator name="Glow"/>
         <Settings>
            <setting id="color" type="QColor" value="#35dfff"/>
            <setting id="cutoutx" type="double" value="95"/>
            <setting id="cutouty" type="double" value="95"/>
            <setting id="firstblurlevel" type="double" value="4"/>
            <setting id="includesource" type="bool" value="true"/>
            <setting id="mode" type="QString" value="Multiply"/>
            <setting id="ontop" type="bool" value="false"/>
            <setting id="secondblurlevel" type="double" value="3"/>
            <setting id="size" type="double" value="3"/>
         </Settings>
         <Sources>
            <source slot="0" source="23"/>
         </Sources>
      </Node>
      <Node id="25" name="Engine fire">
         <pos x="565.716" y="1647.16"/>
         <generator name="Fire"/>
         <Settings>
            <setting id="falloff" type="double" value="0.18"/>
            <setting id="iterations" type="int" value="190"/>
            <setting id="randomize" type="int" value="37"/>
         </Settings>
         <Sources/>
      </Node>
      <Node id="26" name="Wavering flame">
         <pos x="1230.03" y="1776.03"/>
         <generator name="Sine transform"/>
         <Settings>
            <setting id="amplitudeone" type="double" value="2"/>
            <setting id="amplitudetwo" type="double" value="1"/>
            <setting id="angle" type="double" value="90"/>
            <setting id="frequencyone" type="double" value="12"/>
            <setting id="frequencytwo" type="double" value="7"/>
            <setting id="offsetone" type="double" value="0"/>
            <setting id="offsettwo" type="double" value="40"/>
         </Settings>
         <Sources>
            <source slot="0" source="25"/>
         </Sources>
      </Node>
      <Node id="27" name="Shape flame">
         <pos x="1816.39" y="1413.39"/>
         <generator name="Transform"/>
         <Settings>
            <setting id="backgroundcolor" type="QColor" value="#00000000"/>
            <setting id="firstXtiles" type="int" value="1"/>
            <setting id="firstYtiles" type="int" value="1"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="34"/>
            <setting id="rotation" type="double" value="0"/>
            <setting id="secondXtiles" type="int" value="1"/>
            <setting id="secondYtiles" type="int" value="1"/>
            <setting id="xscale" type="double" value="18"/>
            <setting id="yscale" type="double" value="34"/>
         </Settings>
         <Sources>
            <source slot="0" source="26"/>
         </Sources>
      </Node>
      <Node id="28" name="Rocket assembly">
         <pos x="2444.91" y="1247.4"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" type="double" value="100"/>
            <setting id="mode" type="QString" value="Normal"/>
            <setting id="order" type="QString" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="27"/>
            <source slot="1" source="24"/>
         </Sources>
      </Node>
      <Node id="29" name="Launch rocket">
         <pos x="2854.51" y="1064.49"/>
         <generator name="Transform"/>
         <Settings>
            <setting id="backgroundcolor" type="QColor" value="#00000000"/>
            <setting id="firstXtiles" type="int" value="1"/>
            <setting id="firstYtiles" type="int" value="1"/>
            <setting id="offsetleft" type="double" value="-18"/>
            <setting id="offsettop" type="double" value="8"/>
            <setting id="rotation" type="double" value="12"/>
            <setting id="secondXtiles" type="int" value="1"/>
            <setting id="secondYtiles" type="int" value="1"/>
            <setting id="xscale" type="double" value="70"/>
            <setting id="yscale" type="double" value="70"/>
         </Settings>
         <Sources>
            <source slot="0" source="28"/>
         </Sources>
      </Node>
      <Node id="30" name="Final">
         <pos x="3081.84" y="370.656"/>
         <generator name="Blending"/>
         <Settings>
            <setting id="alpha" type="double" value="100"/>
            <setting id="mode" type="QString" value="Normal"/>
            <setting id="order" type="QString" value="Slot 2 on top of Slot 1"/>
         </Settings>
         <Sources>
            <source slot="0" source="16"/>
            <source slot="1" source="29"/>
         </Sources>
      </Node>
      <Node id="31" name="Lower body">
         <pos x="394.267" y="1171.66"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#dff5ff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="14"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="22"/>
            <setting id="rotation" type="double" value="0"/>
            <setting id="width" type="double" value="20"/>
         </Settings>
         <Sources>
            <source slot="0" source="20"/>
         </Sources>
      </Node>
      <Node id="32" name="Cyan star">
         <pos x="1188.65" y="-895.246"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#55f0ff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="2.4"/>
            <setting id="offsetleft" type="double" value="-42"/>
            <setting id="offsettop" type="double" value="4"/>
            <setting id="rotation" type="double" value="45"/>
            <setting id="width" type="double" value="0.4"/>
         </Settings>
         <Sources>
            <source slot="0" source="40"/>
         </Sources>
      </Node>
      <Node id="33" name="Small gold star">
         <pos x="1662.13" y="-874.332"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ffd66b"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="2"/>
            <setting id="offsetleft" type="double" value="-18"/>
            <setting id="offsettop" type="double" value="30"/>
            <setting id="rotation" type="double" value="45"/>
            <setting id="width" type="double" value="0.35"/>
         </Settings>
         <Sources>
            <source slot="0" source="41"/>
         </Sources>
      </Node>
      <Node id="34" name="Small white star">
         <pos x="2120.97" y="-965.17"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ffffff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="2.2"/>
            <setting id="offsetleft" type="double" value="15"/>
            <setting id="offsettop" type="double" value="26"/>
            <setting id="rotation" type="double" value="45"/>
            <setting id="width" type="double" value="0.35"/>
         </Settings>
         <Sources>
            <source slot="0" source="42"/>
         </Sources>
      </Node>
      <Node id="35" name="Lower pink star">
         <pos x="2690.06" y="-922.77"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ff93df"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="3"/>
            <setting id="offsetleft" type="double" value="40"/>
            <setting id="offsettop" type="double" value="38"/>
            <setting id="rotation" type="double" value="45"/>
            <setting id="width" type="double" value="0.45"/>
         </Settings>
         <Sources>
            <source slot="0" source="43"/>
         </Sources>
      </Node>
      <Node id="36" name="Engine nozzle">
         <pos x="1266.7" y="1382.69"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#15214d"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="7"/>
            <setting id="offsetleft" type="double" value="0"/>
            <setting id="offsettop" type="double" value="29"/>
            <setting id="rotation" type="double" value="0"/>
            <setting id="width" type="double" value="12"/>
         </Settings>
         <Sources>
            <source slot="0" source="31"/>
         </Sources>
      </Node>
      <Node id="37" name="Blue star cross">
         <pos x="-163.308" y="-1078.15"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#7ee8ff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="4.2"/>
            <setting id="offsetleft" type="double" value="-38"/>
            <setting id="offsettop" type="double" value="-34"/>
            <setting id="rotation" type="double" value="135"/>
            <setting id="width" type="double" value="0.6"/>
         </Settings>
         <Sources>
            <source slot="0" source="2"/>
         </Sources>
      </Node>
      <Node id="38" name="White star cross">
         <pos x="322.183" y="-1109.72"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ffffff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="2.6"/>
            <setting id="offsetleft" type="double" value="-12"/>
            <setting id="offsettop" type="double" value="-22"/>
            <setting id="rotation" type="double" value="135"/>
            <setting id="width" type="double" value="0.4"/>
         </Settings>
         <Sources>
            <source slot="0" source="3"/>
         </Sources>
      </Node>
      <Node id="39" name="Pink star cross">
         <pos x="771.944" y="-925.095"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ff8cdd"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="3.2"/>
            <setting id="offsetleft" type="double" value="8"/>
            <setting id="offsettop" type="double" value="-40"/>
            <setting id="rotation" type="double" value="135"/>
            <setting id="width" type="double" value="0.5"/>
         </Settings>
         <Sources>
            <source slot="0" source="4"/>
         </Sources>
      </Node>
      <Node id="40" name="Gold star cross">
         <pos x="1889.89" y="-394.009"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ffe18a"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="3.8"/>
            <setting id="offsetleft" type="double" value="36"/>
            <setting id="offsettop" type="double" value="-12"/>
            <setting id="rotation" type="double" value="135"/>
            <setting id="width" type="double" value="0.6"/>
         </Settings>
         <Sources>
            <source slot="0" source="5"/>
         </Sources>
      </Node>
      <Node id="41" name="Cyan star cross">
         <pos x="1087.34" y="-1425.18"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#55f0ff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="2.4"/>
            <setting id="offsetleft" type="double" value="-42"/>
            <setting id="offsettop" type="double" value="4"/>
            <setting id="rotation" type="double" value="135"/>
            <setting id="width" type="double" value="0.4"/>
         </Settings>
         <Sources>
            <source slot="0" source="32"/>
         </Sources>
      </Node>
      <Node id="42" name="Small gold star cross">
         <pos x="1721.89" y="-1438.12"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ffd66b"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="2"/>
            <setting id="offsetleft" type="double" value="-18"/>
            <setting id="offsettop" type="double" value="30"/>
            <setting id="rotation" type="double" value="135"/>
            <setting id="width" type="double" value="0.35"/>
         </Settings>
         <Sources>
            <source slot="0" source="33"/>
         </Sources>
      </Node>
      <Node id="43" name="Small white star cross">
         <pos x="2493.16" y="-1469.6"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ffffff"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="2.2"/>
            <setting id="offsetleft" type="double" value="15"/>
            <setting id="offsettop" type="double" value="26"/>
            <setting id="rotation" type="double" value="135"/>
            <setting id="width" type="double" value="0.35"/>
         </Settings>
         <Sources>
            <source slot="0" source="34"/>
         </Sources>
      </Node>
      <Node id="44" name="Lower pink star cross">
         <pos x="3248.62" y="-982.414"/>
         <generator name="Square"/>
         <Settings>
            <setting id="antialiasing" type="bool" value="true"/>
            <setting id="color" type="QColor" value="#ff93df"/>
            <setting id="cutoutheight" type="double" value="0"/>
            <setting id="cutoutwidth" type="double" value="0"/>
            <setting id="height" type="double" value="3"/>
            <setting id="offsetleft" type="double" value="40"/>
            <setting id="offsettop" type="double" value="38"/>
            <setting id="rotation" type="double" value="135"/>
            <setting id="width" type="double" value="0.45"/>
         </Settings>
         <Sources>
            <source slot="0" source="35"/>
         </Sources>
      </Node>
   </Nodes>
</TextureSet>
