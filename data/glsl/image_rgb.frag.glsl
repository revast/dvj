//image_rgb.frag.glsl

uniform sampler2D texLeft;
uniform sampler2D texCenter;
uniform sampler2D texRight;
uniform float brightnessScalar;
uniform float rgbSpatializerScalar;

void main()
{
	vec4 texColorLeft = texture2D(texLeft,gl_TexCoord[0].st + vec2(-0.01*rgbSpatializerScalar,0));
	vec4 texColorCenter = texture2D(texCenter,gl_TexCoord[0].st);
	vec4 texColorRight = texture2D(texRight,gl_TexCoord[0].st + vec2(0.01*rgbSpatializerScalar,0));
	gl_FragColor.r = gl_Color.r * texColorLeft.r * brightnessScalar;
	gl_FragColor.g = gl_Color.g * texColorCenter.g * brightnessScalar;
	gl_FragColor.b = gl_Color.b * texColorRight.b * brightnessScalar;
	gl_FragColor.a = gl_Color.a;
}

