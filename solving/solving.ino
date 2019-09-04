int ind=0;
int deleteElement(char* arr, int n, int pos){ 
  for (int i = pos; i < n - 1; i++)
    arr[i] = arr[i + 1];
  arr[n-1] = '\0';
  return n - 1;
}
 
int insertElement(char* arr, int n, int pos, int val){
  for (int i = n; i >= pos; i--)
    arr[i] = arr[i - 1];
  arr[pos] = val;
  return n + 1; 
}

char RightHand(char a[]){
  
  if(a[0]=='R' && a[1]=='B' && a[2]=='L'){
    return 'B';
  }
  else if(a[0]=='R' && a[1]=='B' && a[2]=='S'){
    return 'L';
  }
  else if(a[0]=='R' && a[1]=='B' && a[2]=='R'){
    return 'S';
  }
  else if(a[0]=='S' && a[1]=='B' && a[2]=='R'){
    return 'L';
  }
  else if(a[0]=='S' && a[1]=='B' && a[2]=='S'){
    return 'B';
  }
  else if(a[0]=='L' && a[1]=='B' && a[2]=='R'){
    return 'B';
  }
  else{
    return 0; 
  }
}

char LeftHand(char a[]){
  
  if(a[0]=='L' && a[1]=='B' && a[2]=='R'){
    return 'B';
  }
  else if(a[0]=='L' && a[1]=='B' && a[2]=='S'){
    return 'R';
  }
  else if(a[0]=='L' && a[1]=='B' && a[2]=='L'){
    return 'S';
  }
  else if(a[0]=='S' && a[1]=='B' && a[2]=='L'){
    return 'R';
  }
  else if(a[0]=='S' && a[1]=='B' && a[2]=='S'){
    return 'B';
  }
  else if(a[0]=='R' && a[1]=='B' && a[2]=='L'){
    return 'B';
  }
  else{
    return '\0'; 
  }
}

int reduce_path(char *path, int n, int index, char choice){
  //n = strlen(path);
  if(index >=0 && index <=n){
    char splice[] = {path[index], path[index+1], path[index+2]}, ans;
    
    if(choice == 'R')
      ans = RightHand(splice);
    else if(choice == 'L')
      ans = LeftHand(splice);
      
    if(ans!='\0'){
      for(int c =0; c<3; c++)
      n = deleteElement(path, n, index);
      n = insertElement(path, n, index, ans);
    }
  }
  ind++;
}

char* simplified_path(char path[], char choice){
  
  //SSLLSRRRBRRBRRRBSRRLLR
  //SSLLSRRSRBRRRBSRRLLR  
  //SSLLSRRSSRRBSRRLLR  
  //SSLLSRRSSRLRRLLR  
  int n = strlen(path);
  for(int i = 0; i<n; i++){
    reduce_path(path, n, i, choice);
  } 
  return path;
}
char pathr[] = "RBRSLLSRRRBRRBRRRBSRRLLR", pathl[] = "SLBLLLLBLRSBLSLLBSSRLRRLLRRLLBL";
void setup(){
  Serial.begin(38400);
  Serial.println(simplified_path(pathr, 'R'));
  Serial.println(simplified_path(pathl, 'L'));
}
void loop(){
  
}

