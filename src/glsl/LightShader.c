#version 330

// rasterFBO
uniform sampler2D Texture0;
uniform sampler2D Texture1;

// debugTargFBO
uniform sampler2D Texture2;
uniform sampler2D Texture3;

// palFBO
uniform sampler3D Texture4;

// shadowMapFBO
uniform sampler2D Texture5;

uniform bool gridOn;
uniform float gammaVal;
uniform vec3 brushCol;
uniform vec4 brushPos;
uniform vec3 lightPos;
uniform bool testOn3;
// uniform vec3 minBounds;
// uniform vec3 maxBounds;
uniform float cellsPerChunk;
uniform vec2 shadowBias;
uniform vec3 lookAtVec;
uniform int iNumSteps;
uniform float voxelsPerCell;
uniform int cellsPerHolder;
uniform float FOV;
uniform vec2 clipDist;
uniform vec2 bufferDim;
uniform vec3 cameraPos;
uniform vec3 lightVec;

uniform mat4 modelviewInverse;
uniform mat4 lightSpaceMatrix;
uniform mat4 pmMatrix;

^INCLUDE:MATERIALS^

^INCLUDE:RayFuncs^




$

layout(location = 0) in vec4 vposition;
layout(location = 1) in vec4 data0;

out vec4 worldPos;

void main() {
    
    worldPos = vec4(vposition.xyz,1.0);
    gl_Position = worldPos;
    
}

$

in vec4 worldPos;

layout(location = 0) out vec4 FragColor0;



float offV[2] = float[](
  10.0,
  10.0
);

float minRad[2] = float[](
  1.0,
  16.0
);
float maxRad[2] = float[](
  32.0,
  128.0
);



vec2 pack16(float num)
{

  int iz = int(num);
  int ig = (iz) / 256;
  int ir = iz - ig*256;//intMod(iz, 256);
  

  vec2 res;

  res.r = float(ir) / 255.0;
  res.g = float(ig) / 255.0;

  return res;

}

float unpack16(vec2 num)
{
  return num.r * 255.0 + num.g * 65280.0;
}

vec3 unpackColor(vec2 num, float lightVal)
{
  return texture( Texture4, vec3(lightVal, num.r, num.g + 0.5/255.0) ).rgb;
}

vec3 rgb2hsv(vec3 c)
{
  vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
  vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
  vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

  float d = q.x - min(q.w, q.y);
  float e = 1.0e-10;
  return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}


vec3 hsv2rgb(vec3 c)
{
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


vec3 getModCol(float lightRes, vec3 normVec, vec3 rd) {
  
  
  float facingCam = (dot(normVec,lookAtVec) + 1.0)/2.0;
  float modVal = pow(1.0-lightRes,10.0)*facingCam;
  // modColor = hsv2rgb(vec3(
  //  mix(0.75, 0.4, modVal)
  //  ,1.0,1.0))*modVal;
  vec3 modColor = vec3(0.0,0.65,1.0)*modVal*1.5;
  
  modVal = pow(lightRes,10.0)*(1.0-facingCam);
  // modColor += hsv2rgb(vec3(
  //  mix(0.0, 0.12, modVal)
  //  ,mix(0.75,1.0,1.0-modVal),1.0))*modVal;
  modColor += (vec3(1.0,0.4,0.0)*modVal + modVal*modVal)*0.5;
  
  modColor += clamp(dot(normVec,rd)+0.75, 0.0,1.0)*vec3(0.65,0.0,1.0)*0.4;
  
  return modColor;
}

float calcShadowSS(vec3 begWorldPos, vec3 normVec) {
  
  
  float rayDis = 4.0;
  int numShadSteps = 32;
  
  
  
  vec2 offsetCoord = vec2(0.0);
  vec2 curTC = vec2(2.0);
  
  vec4 curRayPos = vec4(0.0);
  vec4 curWorldPos = vec4(0.0);
  
  int i;
  float fi;
  
  
  float fNumShadSteps = float(numShadSteps);
  
  float lerpVal = 0.0;
  float hitCount = 0.0;
  float totCount = 0.0;
  
  vec4 transPos = vec4(0.0);
  vec3 projCoords = vec3(0.0);
  
  vec4 begPosWS = vec4(begWorldPos.xyz,0.0);
  vec4 endPosWS = vec4(begWorldPos.xyz,0.0);
  
  endPosWS.xyz += -lightVec.xyz*rayDis;
  
  begPosWS.w = distance(cameraPos.xyz,begPosWS.xyz);
  endPosWS.w = distance(cameraPos.xyz,endPosWS.xyz);
  
  float bias = clamp((dot(normVec, lightVec)+1.0)*0.5,0.0,1.0);
  float newBias = mix(-0.1, 0.1, bias);
  
  for (i = 0; i < numShadSteps; i++) {
    fi = float(i);
    lerpVal = fi/fNumShadSteps;
    
    curRayPos = mix(begPosWS, endPosWS, lerpVal);
    
    transPos = pmMatrix*vec4(curRayPos.xyz,1.0);
    projCoords = transPos.xyz / transPos.w;
    
    if ((abs(projCoords.x) < 1.0) && (abs(projCoords.y) < 1.0)) {
      projCoords = projCoords * 0.5 + 0.5;
      
      curTC = projCoords.xy;
      
      
      curWorldPos.xyz = texture(Texture0,curTC).xyz;
      curWorldPos.w = distance(cameraPos.xyz,curWorldPos.xyz);
      
      
      //if (curWorldPos.w < (curRayPos.w+newBias)) {
      //  hitCount += 1.0;
      //}
      
      hitCount += clamp( ((curRayPos.w+newBias)-curWorldPos.w)*4.0, 0.0,1.0);
      
      
    }
    totCount += 1.0;
    
    
  }
  
  return 1.0-clamp(hitCount/totCount,0.0,1.0);
  
  
}

float calcAO(vec2 texc, vec3 worldPosition, vec3 normVec) {
  
  float totHits = 0.0;
  float totRays = 0.0;

  float curMin = 0.0;
  float curMax = 0.0;
  float curOff = 0.0;
  
  float curRad = 0.0;
  float minRotInc = 3.14159;
  float maxRotInc = 0.01;
  float curRotInc = 0.0;
  float curRot = 0.0;
  
  float curAO = 0.0;
  
  int i;
  int j;
  
  float fi;
  
  vec4 samp;
  
  vec2 offsetCoord = vec2(0.0);
  vec2 newTC = vec2(2.0);

  vec3 testVec = vec3(0.0);

  float camDis2 = distance(worldPosition.xyz,cameraPos.xyz);

  float ssao = 1.0;
  
  float fNumSteps = float(iNumSteps);
  

  for (j = 0; j < 2; j++)
  {
    totHits = 0.0;
    totRays = 0.0;

    curMin = minRad[j];
    curMax = maxRad[j];
    curOff = offV[j]/128.0; // todo: change this based on RAY MODE

    for (i = 0; i < iNumSteps; i++)
    {

      fi = float(i) / fNumSteps;

      curRad = mix(curMin, curMax, fi);
      curRotInc = mix(minRotInc, maxRotInc, fi);

      offsetCoord.x = cos(curRot) * curRad;
      offsetCoord.y = sin(curRot) * curRad;

      newTC = texc.xy + (offsetCoord) / (bufferDim);

      // samp1 = texture(Texture0, newTC );
      // samp2 = texture(Texture0, newTC );
      
      samp = texture(Texture0, newTC );//mix(samp1,samp2,float(samp1.w > samp2.w));
      
      testVec = normalize(samp.xyz - (worldPosition.xyz + normVec * curOff));
      curAO = clamp ( clamp(  ( dot(testVec, normVec) ), 0.0, 1.0), 0.0, 1.0)*clamp(
        1.0-distance(worldPosition.xyz,samp.xyz)/camDis2, 0.0, 1.0  
      );
      totHits += curAO;
      
      
      totRays += 1.0;

      curRot += curRotInc;
    }
    
    curAO = clamp(1.0-( totHits*2.0 / totRays), 0.0, 1.0);
    
    ssao *= curAO;
    
  }
  
  
  
  return clamp(ssao, 0.0, 1.0);
}

// vec4 worldToScreen(vec4 wp) {
//   vec4 screenPos = wp;
  
//   screenPos.xyz -= minBounds;
//   screenPos.xyz /= (maxBounds-minBounds);
  
//   float fd = 1.0-screenPos.z;
  
//   screenPos.xy += lightVec.xy*(fd);
  
  
  
//   return screenPos;
// }

//bool globBool;


float getGrid(vec3 worldPosition, vec3 normVec) {
  
  vec3 absNorm = abs(normVec);
  
  float cellSize = 1.0;//cellsPerChunk;
  vec3 grid0 = 
      //floor(worldPosition.xyz/cellSize);
      abs(mod(worldPosition.xyz, cellSize) - cellSize / 2.0) * 2.0;
  
  float unitBuf = (cellSize - cellSize/8.0);
  
  float gridVal0 = clamp(
      (
        float(grid0.x >= unitBuf)*(1.0-absNorm.x) +
        float(grid0.y >= unitBuf)*(1.0-absNorm.y) +
        float(grid0.z >= unitBuf)*(1.0-absNorm.z)  
      )*(1.0-distance(cameraPos.xyz,worldPosition.xyz)/30.0)
      , 0.0, 1.0
  );
  
  return gridVal0;
}



float calcShadow(vec4 worldPos, vec4 worldPosInLightSpace, vec3 normVec) {
    // perform perspective divide
    vec3 projCoords = worldPosInLightSpace.xyz / worldPosInLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    
    float cutoff = projCoords.z;
    
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(Texture5, projCoords.xy).w; 
    // Get depth of current fragment from light's perspective
    //projCoords.z *= 0.5;
    float currentDepth = distance(lightPos.xyz,worldPos.xyz);//projCoords.z;
    // Calculate bias (based on depth map resolution and slope)
    float bias = clamp((dot(normVec, lightVec)+1.0)*0.5,0.0,1.0);
    currentDepth += mix(shadowBias.x, shadowBias.y, 1.0-bias);
    
    
    // Check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    
    int x = 0;
    int y = 0;
    float pcfDepth = 0.0;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(Texture5, 0);
    for(x = -1; x <= 1; ++x)
    {
        for(y = -1; y <= 1; ++y)
        {
            pcfDepth = texture(Texture5, projCoords.xy + vec2(x, y) * texelSize).w; 
            shadow += float(currentDepth < pcfDepth);        
        }
    }
    shadow /= 9.0;
    
    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(cutoff > 1.0) {
      shadow = 0.0;
      //globBool = true;
    }
    
    return clamp(1.0-shadow,0.0,1.0);
}

vec3 doGamma(vec3 t, float gv) {
  vec3 oneVec = vec3(1.0);
  vec3 twoVec = vec3(2.0);
  vec3 tInv = (oneVec-t);
  
  float p0 = 0.0;
  float p1 = gv;
  float p2 = 1.0;
  
  
  return 
    tInv*tInv*p0 +
    twoVec*tInv*t*p1 +
    t*t*p2;
  
  // x = 
  //   (1 - t) * (1 - t) * p[0].x +
  //   2 * (1 - t) * t * p[1].x +
  //   t * t * p[2].x;
  // y = (1 - t) * (1 - t) * p[0].y + 2 * (1 - t) * t * p[1].y + t * t * p[2].y;
}

void main() {

  //globBool = false;

  vec4 oneVec = vec4(1.0);
  vec2 TexCoord0 = gl_FragCoord.xy/(bufferDim.xy);
  
  
  
  
  vec2 borderCoords = abs(TexCoord0.xy*2.0-1.0);
  borderCoords *= bufferDim.xy;
  vec2 border = bufferDim.xy-32.0;
  if (
    (borderCoords.x > border.x) ||
    (borderCoords.y > border.y)
  ) {
    FragColor0 = vec4(
      0.0,0.0,0.0,1.0
    );
    return;
  }
  
  
  
  
  
  vec3 ro = vec3(0.0);
  vec3 rd = vec3(0.0);
  getRay(TexCoord0,ro,rd);
  
  float camDis = clipDist.y;
  
  vec4 tex0 = texture(Texture0,TexCoord0.xy);
  vec4 tex1 = texture(Texture1,TexCoord0.xy);
  
  vec4 tex2 = texture(Texture2,TexCoord0.xy);
  vec4 tex3 = texture(Texture3,TexCoord0.xy);

  if (testOn3) {
    FragColor0 = vec4(
     vec3(
      distance(texture(Texture5,TexCoord0.xy).xyz,cameraPos.xyz)/600.0  
     ),
     //(sin(texture(Texture5,TexCoord0.xy).xyz)+1.0)*0.5,
     1.0
    );
    return;
  }
  
  vec3 oldCol = vec3(0.0);
  
  vec4 shadowSamp = vec4(0.0);
  vec4 screenPos = vec4(0.0);

  float lightValOrig = clamp(dot(tex1.xyz,-lightVec.xyz),0.0,1.0);
  float lightValOrigN = clamp(dot(tex1.xyz,lightVec.xyz),0.0,1.0);
  float lightVal;
  float aoVal = 0.0;
  vec3 finalCol;

  vec3 fogCol = vec3(TexCoord0.y,0.8,1.0);
  float lightRes = 0.0;
  
  float fogDis = clamp((distance(cameraPos,tex0.xyz)*2.0-32.0)/clipDist.y,0.0,1.0);
  float shadowVal = 0.0;
  float shadowVal2 = 0.0;
  
  vec4 matVals = vec4(0.0);

  //vec2 curTex = vec2(TEX_EARTH,0.0);
  float curMat = tex1.w;//floor(curTex.x*256.0*255.0) + floor(curTex.y*255.0);

  vec4 worldPosInLightSpace = vec4(0.0);

  if (dot(oneVec,tex0) == 0.0) {
    finalCol = fogCol;
  }
  else {
    
    aoVal = calcAO(TexCoord0, tex0.xyz, tex1.xyz);
    //aoVal = sqrt(aoVal);
    
    worldPosInLightSpace = lightSpaceMatrix*vec4(tex0.xyz,1.0);
    shadowVal = calcShadow(tex0, worldPosInLightSpace, tex1.xyz);
    shadowVal2 = calcShadowSS(tex0.xyz, tex1.xyz);
    shadowVal *= shadowVal2;
    lightVal = lightValOrig*shadowVal;
    
    lightRes = 
      clamp(mix(
        (aoVal)*0.25 + lightValOrigN*0.2,lightVal,lightVal  
      ),0.0,1.0) 
    ;
    
    
    matVals = vec4(0.0,0.0,pack16(curMat));
    //matVals.b = 0.0;
    finalCol = unpackColor(matVals.ba,lightRes); //*vec2(aoVal,1.0)
    
    oldCol = finalCol;
    
    finalCol = mix(finalCol*0.5,finalCol,aoVal);
    finalCol = sqrt(finalCol);
    finalCol = mix(finalCol*finalCol*finalCol,finalCol,finalCol);
    
    //finalCol += vec3(1.0-shadowVal2,0.0,0.0);
    //finalCol = vec3(shadowVal2);
    
    //finalCol = vec3(aoVal);
    
    finalCol += getModCol(lightRes, tex1.xyz, rd)*mix(0.2,0.5,lightRes);//(lightVal*0.25+0.25);
    
    
    //finalCol = vec3(matVals.b);
    
    //finalCol = oldCol;
    
    //finalCol = mix(finalCol*finalCol,sqrt(finalCol),finalCol);
    
    //
    
    //finalCol += vec3(shadowVal2,0.0,0.0);
    
    //finalCol = vec3(shadowVal2);
    
    // if (distance(matVals.a,TEX_GRASS) < 0.5/255.0) {
    //   finalCol *= aoVal;
    // }
    
    
    
    // screenPos = worldToScreen(vec4(tex0.xyz,1.0));
    // if (
    //   all(greaterThan(screenPos.xy,vec2(0.0))) &&
    //   all(lessThan(screenPos.xy,vec2(1.0)))
    // ) {
    //   shadowSamp = texture(Texture5,screenPos.xy+0.5);
    //   if ((shadowSamp.z+0.5) > tex0.z) {
    //     finalCol *= 0.25;
    //   }
    // }
    
    //finalCol.r += shadowVal;
    
    //finalCol = vec3(shadowVal2);
    
    //finalCol = vec3(shadowVal2);
    
    camDis = distance(cameraPos.xyz,tex0.xyz);
    
    if (gridOn) {
      finalCol += getGrid(tex0.xyz,tex1.xyz)*vec3(0.1);
    }
    
    finalCol = mix(
      finalCol,//mix(finalCol*0.25,finalCol,shadowVal),
      fogCol,
      fogDis
    );
    
    //finalCol = vec3(shadowVal);
    
    // if (globBool) {
    //   finalCol = vec3(1.0,0.0,1.0);
    // }

  }

  if (distance(cameraPos.xyz,tex2.xyz) < distance(cameraPos.xyz,tex0.xyz)) {
    if (dot(tex2.xyz,oneVec.xyz) != 0.0) {
      finalCol = tex3.xyz;
    }
  }
  
  //
  
  // vec2 boxVal;
  // vec3 boxPos;
  // float camDisBox;
  
  // if (brushPos.w == 0.0) {
    
  // }
  // else {
  //   boxVal = aabbIntersect(ro,rd,brushPos.xyz-brushPos.w,brushPos.xyz+brushPos.w);
    
    
    
  //   if (boxVal.x <= boxVal.y) {
  //     boxPos = ro + rd*boxVal.x;
  //     camDisBox = distance(cameraPos.xyz,boxPos.xyz);
      
  //     if (camDisBox < camDis) {
  //       finalCol = mix(finalCol,brushCol,0.5);
  //     }
  //   }
  // }
  
  finalCol.xyz = doGamma(finalCol.xyz,gammaVal);

  //mod(tex.xyz+0.01,1.0);
  


  FragColor0 = vec4(
    finalCol.xyz,
    1.0
  );

}









