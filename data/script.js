$(document).ready(function(){
    $("#btnvalvit").click(function(){
        var valeur = $("#selectvit").val();
        $.post("vitesse",{
            valeurvitesse: valeur
        });
    });
});

setInterval(function getmode()
{
    var xhttp = new XMLHttpRequest();
    var mode

   
    xhttp.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
            mode = this.responseText;
            if(mode =="Automatique"){
                document.getElementById("valeurmode").innerHTML="Automatique";
                document.getElementById("btnavancer").style.visibility='hidden';
                document.getElementById("btnstop").style.visibility='hidden';
                document.getElementById("btnreculer").style.visibility='hidden';
                document.getElementById("btnvider").style.visibility='hidden';
                document.getElementById("btnstartstop").style.visibility='visible';
            }
            else{
                document.getElementById("valeurmode").innerHTML="Manuel";
                document.getElementById("btnstop").style.visibility='visible';
                document.getElementById("btnvider").style.visibility='visible';
                document.getElementById("btnreculer").style.visibility='visible';
                document.getElementById("btnavancer").style.visibility='visible';
                document.getElementById("btnstartstop").style.visibility='hidden';
            }
        
        }
    };

    

    xhttp.open("GET", "synchromode", true);
    xhttp.send();

}, 2000);

setInterval(function getmode()
{
    var xhttp = new XMLHttpRequest();
    var NiveauDeBatterie

   
    xhttp.onreadystatechange = function()
    {
        if(this.readyState == 4 && this.status == 200)
        {
            NiveauDeBatterie = this.responseText;
            document.getElementById("valeurbat").innerHTML=NiveauDeBatterie;
        
        }
    };

    

    xhttp.open("GET", "getbatterie", true);
    xhttp.send();

}, 60000);



function autobtn(){
    document.getElementById("valeurmode").innerHTML="Automatique"
    document.getElementById("btnavancer").style.visibility='hidden'
    document.getElementById("btnstop").style.visibility='hidden'
    document.getElementById("btnreculer").style.visibility='hidden'
    document.getElementById("btnvider").style.visibility='hidden'
    document.getElementById("btnstartstop").style.visibility='visible'

    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "auto", true);
    xhttp.send();

}

function manubtn(){
    document.getElementById("valeurmode").innerHTML="Manuel";
    document.getElementById("btnstop").style.visibility='visible';
    document.getElementById("btnvider").style.visibility='visible';
    document.getElementById("btnreculer").style.visibility='visible';
    document.getElementById("btnavancer").style.visibility='visible';
    document.getElementById("btnstartstop").style.visibility='hidden';

    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "manu", true);
    xhttp.send();
}

function startstopbtn(){
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "startstop", true);
    xhttp.send();
}

function avancerbtn(){
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "avancer", true);
    xhttp.send();
}

function stopbtn(){
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "stop", true);
    xhttp.send();
}

function reculerbtn(){
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "reculer", true);
    xhttp.send();
}

function viderbtn(){
    var xhttp = new XMLHttpRequest();
    xhttp.open("GET", "vider", true);
    xhttp.send();
}


