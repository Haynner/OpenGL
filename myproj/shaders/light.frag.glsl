#version 330 core

uniform int myrenderStyle;
uniform mat4 myview_matrix;
uniform mat4 myprojection_matrix;
uniform mat3 mynormal_matrix;
uniform mat4 mymodel_matrix;

uniform vec4 mylightPosition;
uniform vec4 mylightColor;
uniform vec3 mylightDirection;
uniform int mylightType;
uniform sampler2D tex;
uniform sampler2D bump;

uniform vec4 mySpotPosition;
uniform vec4 mySpotColor;
uniform vec3 mySpotDirection;

in vec4 vectex_to_fragment;
in vec3 normal_to_fragment;
in vec2 texture_to_fragment;
in vec3 tangent_to_fragment;


void main (void)
{   
	//Position
	vec4 position_ = myview_matrix * mymodel_matrix * vectex_to_fragment;
	vec3 position = (position_.xyz / position_.w);

	//Initialisation kd si pas de texture;
	vec4 kd = vec4(0,0,0,0);
	
	//Normal
	vec3 normal = mynormal_matrix * normal_to_fragment;
	normal += mynormal_matrix*(2.0 * texture2D(bump, texture_to_fragment.st).rgb - 1.0);

	normal=normalize(normal);

	vec3 t = normalize(mynormal_matrix * tangent_to_fragment);
	vec3 b = normalize(cross(normal,t));
	mat3 in_m = mat3(t,b,normal);
	mat3 out_m = transpose(in_m);

	//origine
	vec3 eye = vec3(0,0,0);
	eye = normalize(out_m * eye);
	

	//Lumiere position
	vec4 lightpos_ = myview_matrix *mylightPosition;
	vec3 lightpos = (lightpos_.xyz) / lightpos_.w;
	vec4 spotpos_ = myview_matrix *mySpotPosition;
	vec3 spotpos = (spotpos_.xyz) / spotpos_.w;

	// Kd texture
	kd+=texture2D(tex, texture_to_fragment.st);

	// Ks
	vec4 ks = vec4(1,1,1,0);

	

	//gl_FragColor = kd * 0.05;

	//Lumiere piece
	vec3 moiVersLumiere = normalize(lightpos - position);
	vec3 moiVersOrigine = normalize(eye - position);

	/*//diffuse
	gl_FragColor += mylightColor * kd * max ( dot(normal,moiVersLumiere), 0.0);
	
	//specular
	vec3 reflected_ray = normalize(reflect(position-lightpos,normal));
	//gl_FragColor += mylightColor * ks * pow(max(dot(reflected_ray,moiVersOrigine),0.0),20);
	*/
	//Spot bleu
	//diffuse
	vec4 color = mySpotColor * pow(max(dot(normalize(mynormal_matrix *mySpotDirection),normalize(position - spotpos)),0),10);
	gl_FragColor += color * kd * max(dot(normal,normalize(spotpos - position)),0.0);

	//reflected_ray = normalize(reflect(position-lightpos,normal));
	//gl_FragColor += color * ks * pow(max(dot(reflected_ray,normalize(eye-position),0.0),20);
}