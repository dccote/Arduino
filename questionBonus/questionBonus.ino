void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.print("\n\nBonjour:\n");
  Serial.print("Vous y êtes presque!!!\n\n");
  Serial.print("Lors de l'ouragan Harvey, un hacker Canadien a aidé l'armée Américaine en programmant\n");
  Serial.print("un système de geolocalisation en temps réel, basé sur les données des Tweets des gens en détresse.\n");
  Serial.print("Il a reçu une lettre de reconnaissance de la part de l'armée Américaine, un article\n");
  Serial.print("a été écrit sur lui dans FastCompany et il a reçu la médaille des Grands Canadiens\n");
  Serial.print("des mains de la gouverneure générale le 1er juillet 2020.\n");
  Serial.print("Quel est son nom et quel était le titre de son mémoire de maitrise?\n");
  Serial.print("\n\nEnvoyez vos réponses à dccote@cervo.ulaval.ca\n");
  delay(3000);
}
