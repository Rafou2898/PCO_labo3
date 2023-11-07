# Rapport du labo 3 PCO : Factory

#### Auteurs : Rafael Dousse et Massimo Stefani

## Description de l'implémentation

Dans ce laboratoire, l'idée est de simuler des chaînes de production de robots, depuis la matière première jusqu’au
produit. Pour ce faire, nous devons implémenté des fonctions les classes `Factory`, `Wholesaler` et `Extractor`. De plus, un problème existe lorsque l'on ferme la fenêtre (UI) et donc il a fallu trouvé une solution pour pas devoir forcer le programme à ce fermer. Les différente fonctions sont appelées de manière concurente et donc pour ne pas avoir des problèmes il faut protéger les bonnes sections critiques.

### Extractor

### Factory

### Wholeseller

### Util
C'est dans ce fichier que toute les différents seller de notre programmes sont crées et terminés. Lorsqu'on clique sur la croix de la fenêtre pour arrêter les différents threads, on doit ajouter dans la fonction `endService()` ce bout de code:
```c++
    for(auto& thread : threads){
        thread->requestStop();
    }
```
Cette boucle for fait un requestStop aux thread et nous permet aussi d'utiliser un booléen dans le while des différentes fonction run() qui sont implémentées dans les `seller`. C'est donc cette condition `!PcoThread::thisThread()->stopRequested())` qui interrompt les boucles while et qui fait que nos threads s'arrêtent de travailler et que l'on peut quitter le programme sans erreur.

## Tests
Pour ce labo, il a été difficile de trouver de bons tests pour vérifier le fonctionnement de notre programme. Cependant, il est quand même possible de faire certains test "visuel". Nous avons par exemple enlever les mutex dans les fonctions et avons vu que < a tester et dire ce que ça fait, peut être ajouter des images>.
Les `Pco usleep` permet aux threads de prendre leur temps pour faire les transactions et construire les différents objets
