(function(){
    const config = {
        apiKey: "AIzaSyBC0bhkyJhIgNI4rNir1DXuvhkhgABkcpA",
        authDomain: "base-hc-sr04.firebaseapp.com",
        databaseURL: "https://base-hc-sr04-default-rtdb.firebaseio.com",
        projectId: "base-hc-sr04",
        storageBucket: "base-hc-sr04.appspot.com",
        messagingSenderId: "1030379136561",
        appId: "1:1030379136561:web:a4cfc60d9dcf959dbc9710",
        measurementId: "G-JXNQPNPNXN"
      };
    firebase.initializeApp(config);

    //pasamos dato a html
    const Nivel_Actual = document.getElementById('nivelActual');
    const Temperatura = document.getElementById('Temperatura');

    //referencia a la base de datos
    const dbrefNivel = firebase.database().ref().child('Nivel_Actual');
    const dbrefTemperatura = firebase.database().ref().child('Temperatura');
    const dbrefModo = firebase.database().ref().child('Modo');
    const dbrefLlenado = firebase.database().ref().child('Llenado');
    const dbrefDrenado = firebase.database().ref().child('Drenado');
    const dbrefLleno = firebase.database().ref().child('Lleno');
    const dbrefaltura = firebase.database().ref().child('/Parametros/altura');
    const dbrefarea = firebase.database().ref().child('/Parametros/area');
    const dbrefguarda = firebase.database().ref().child('/Parametros/guarda');
    const dbrefNivelD = firebase.database().ref().child('Nivel_Deseado')
    const dbrefSistema = firebase.database().ref().child('Sistema');Sistema
    dbrefguarda.on('value', snap => {
      document.configuracion.guarda.value=snap.val();
    });
    dbrefarea.on('value', snap => {
      document.configuracion.area.value=snap.val();
    });
    dbrefaltura.on('value', snap => {
      document.configuracion.altura.value=snap.val();
    });
    
    

    //escuchar cambios en la base de datos
     
    dbrefNivel.on('value', snap => {
      Nivel_Actual.innerText = snap.val();
      Nivel=snap.val();
      if ((Nivel>=0)&&(Nivel<=20)){
        document.getElementById("Item-Nivel").style.backgroundColor="#0cc0df";
      }else{ 
        if ((Nivel>20)&&(Nivel<=30)){
          document.getElementById("Item-Nivel").style.backgroundColor="#055ab5";
        }else{ 
          document.getElementById("Item-Nivel").style.backgroundColor="#160096";
        }
      }
    });

   
    dbrefTemperatura.on('value', snap => {
      Temperatura.innerText = snap.val();
      Tem=snap.val();
      if ((Tem>=0)&&(Tem<=30)){
        document.getElementById("Item-Temp").style.backgroundColor="green";
      }else{ 
        if ((Tem>30)&&(Tem<=40)){
          document.getElementById("Item-Temp").style.backgroundColor="yellow";
        }else{ 
          document.getElementById("Item-Temp").style.backgroundColor="red";
        }
      }
    });



      //Conmutacion de Modo
    $("#Modo").click(function(){
      var estado = $(this).is(':checked');
      let element = document.getElementById("slider");
      if (estado) {
        dbrefModo.set(!estado);
        element.setAttribute("hidden", "hidden");
      } else {
        dbrefModo.set(!estado);
        element.removeAttribute("hidden");
      }
    });
    //Conmutacion de Sistema
    $("#Sistema").click(function(){
      var estado = $(this).is(':checked');
      if (estado) {
        dbrefSistema.set(!estado);
      } else {
        dbrefSistema.set(!estado);
      }
    });

    //Configuracion de colores de las bombas
    dbrefLleno.on('value', snap => {
      const Lleno = snap.val();
      if (Lleno) {
        document.getElementById("Llenado").style.backgroundColor="white";
        document.getElementById("Drenado").style.backgroundColor="white";
        document.getElementById("Lleno").style.backgroundColor="green";  
      }else{
            dbrefLlenado.on('value', snap => {
            const Llenado = snap.val();
            if (Llenado) {
              document.getElementById("Llenado").style.backgroundColor="green";
              document.getElementById("Drenado").style.backgroundColor="white";
              document.getElementById("Lleno").style.backgroundColor="white";
            }else{
              dbrefDrenado.on('value', snap => {
                const Drenado = snap.val();
                if (Drenado) {
                  document.getElementById("Llenado").style.backgroundColor="white";
                  document.getElementById("Drenado").style.backgroundColor="green";
                  document.getElementById("Lleno").style.backgroundColor="white";
                }else{
                  document.getElementById("Llenado").style.backgroundColor="white";
                  document.getElementById("Drenado").style.backgroundColor="white";
                  document.getElementById("Lleno").style.backgroundColor="white";
                }
              });
            }
          });
      }
    });

    //Configuracion de temperatura
    addEventListener('load',inicio,false);

    function inicio()
    {
      document.getElementById('temperatura').addEventListener('change',cambioTemperatura,false);
    }
  
    function cambioTemperatura()
    {    
      document.getElementById('temp').innerHTML=(document.getElementById('temperatura').value)+"%";
      dbrefNivelD.set(parseFloat(document.getElementById('temperatura').value));
    }

}());

//Configuracion de tanque
function configurar(){
  const dbrefaltura = firebase.database().ref().child('/Parametros/altura');
  const dbrefarea = firebase.database().ref().child('/Parametros/area');
  const dbrefguarda = firebase.database().ref().child('/Parametros/guarda');
  var area=parseFloat(document.configuracion.area.value);
  var altura=parseFloat(document.configuracion.altura.value);
  var guarda=parseFloat(document.configuracion.guarda.value);
  dbrefarea.set(area);
  dbrefaltura.set(altura);
  dbrefguarda.set(guarda);
}

//Login
function ingresar() {
  var email = document.getElementById('email2').value;
  var contrasena = document.getElementById('contrasena2').value;
  let element = document.getElementById("body");
  firebase.auth().signInWithEmailAndPassword(email, contrasena)
      .then((user) => {
          // Signed in
          // ...
          console.log('sesión iniciada');
          element.setAttribute("display", "grid");
      })
      .catch((error) => {
          var errorCode = error.code;
          var errorMessage = error.message;
          element.setAttribute("display", "none");
      });
}


//Registro
function registrar() {
  var email = document.getElementById('email').value;
  var contrasena = document.getElementById('contrasena').value;

  firebase.auth().createUserWithEmailAndPassword(email, contrasena)
      .then((user) => {
          console.log('Usuario registrado!');
      })
      .catch((error) => {
          var errorCode = error.code;
          var errorMessage = error.message;           
          console.log(errorMessage);
      });
}