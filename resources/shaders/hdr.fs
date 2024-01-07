     #version 330 core
     out vec4 FragColor;

     in vec2 TexCoords;

     uniform sampler2D hdrBuffer;
     uniform sampler2D bloomBlur;
     uniform bool hdr;
     uniform bool bloom;
     uniform float exposure;

     void main()
     {
         const float gamma = 2.2;
         vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;
         vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;


         if (bloom) 
                hdrColor += bloomColor;//do additive blending if bloom is enabled
             

        vec3 result = hdrColor;
         if(hdr)
         {
	     vec3 result = hdrColor / (hdrColor + vec3(1.0));//tone mapping
             result = vec3(1.0) - exp(-hdrColor * exposure);// exposure 
             result = pow(result, vec3(1.0 / gamma));//gamma correction 
             FragColor = vec4(result, 1.0);
         }
         else
         {
             vec3 result = pow(hdrColor, vec3(1.0 / gamma));//gamma correction 
             FragColor = vec4(result, 1.0);
         }
     }
