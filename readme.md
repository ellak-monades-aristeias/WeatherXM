## Weather ex Machina##

Στόχος του έργου είναι να κατασκευάσουμε ένα πρωτότυπο μετεωρολογικό σταθμό, ανταγωνιστικό προς τους υφιστάμενους ‘παραδοσιακούς’, μεγάλου κόστους μετεωρολογικούς σταθμούς. Ο σταθμός αυτός θα συμβάλλει στην υπέρβαση του προβλήματος διαθεσιμότητας μετεωρολογικών δεδομένων, μέσω της γεωγραφικής ανάπτυξης μεγάλου αριθμού αυτόνομων συνόλων αισθητήρων. Το υλισμικό θα είναι χαμηλού κόστους (<100€), με σύνδεση WiFi σε αστικό περιβάλλον και δυνατότητα για 2G/3G M2M για χρήση σε απομακρυσμένες περιοχές.

**Ακολουθούν τα χαρακτηριστικά στα οποία στοχεύουμε:**
 - Συμπαγής, μικρού μεγέθους, εύκολης τοποθέτησης χωρίς απαραίτητη χρήση ιστού (π.χ. θα μπορούσε να κρέμεται σε ένα παράθυρο/μπαλκόνι/γωνία)
 - χαμηλό συνολικό κόστος (λιγότερο από 100€) 
 - Mε αισθητήρες θερμοκρασίας, υγρασίας, ατμοσφαιρικής πίεσης, βροχής και αέρα. Για λόγους κόστους δε θα μετράμε βροχόπτωση, αλλά μόνο ύπαρξη βροχής με αισθητήρες σταγόνας. Αντίστοιχα δε θα μετράμε κατεύθυνση/ένταση αέρα αλλά από την κίνηση του ιδίου του σώματος του κουτιού, εφόσον κρέμεται από ένα κορδόνι, θα καταλαβαίνουμε την άπνοια ή ύπαρξη αέρα 
 - Aυτόνομο ενεργειακά, με την χρήση επαναφορτιζόμενων μπαταριών (LiPo or NiMH) που θα φορτίζουν από ένα μικρο ενσωματωμένο φωτοβολταικο
- Διασύνδεση WiFi και αποστολή δεδομένων μετρήσεων σε server μέσω (HTTP or MQTT)
 - Δυνατότητα ενημέρωσης firmware και φόρτιση μπαταρίας μέσω USB
 - Φιλικό προς DIY κατασκευαστές, συμβατό με Arduino  λογισμικό και έτοιμους αισθητήρες / modules / breakout boards
 - Δυνατότητα λειτουργίας από παραδοσιακό παραλληλεπίπεδο πλαστικό κουτί, μελέτη/προετοιμασία για περίβλημα/κουτί 3D εκτύπωσης, για οταν τα ηλεκτρονικά θα είναι σε custom PCB
 - Δυνατότητα over the Air firmware update
 - Δυνατότητα προσθήκης module για GPS/GALILEO, 3G with M2M Sim, Bluetooth
 - Δυνατότητα σύνδεσης εσωτερικής μονάδας με οθόνη για άμεση εμφάνιση των μετρήσεων


Πιο αναλυτικά οι εργασίες σε αυτή την φάση είναι:
 **1. Αξιολόγηση υποψήφιου υλισμικού με υλοποίηση ελάχιστου συνδεδεμένου συστήματος (“hello world” prototype)**
Έχουμε εντοπίσει αρκετές πλατφόρμες IoT με έτοιμα modules αισθητήρων, οι οποίες είναι συμβατές με arduino και φιλικές για prototyping. Για παράδειγμα:
 - TI CC3200
 - Spark/Particle Core,Photon
 - Arduino Yun
 - AVR & ESP8266
 - Libelium waspmote
 - LinkIt ONE

Είναι εξαιρετικά δύσκολο να καταλήξει κανείς στην επιλογή μιας IoT πλατφόρμας χωρίς σε  βάθος προηγούμενη εμπειρία με τη συγκεκριμένη οικογένεια hardware/software. Για το λόγο αυτό και ακολουθώντας μεθοδολογίες agile, θα αναπτύξουμε ένα ελάχιστο, αλλά ολοκληρωμένο σύστημα σε τουλάχιστον τρεις (3) από τις παραπάνω πλατφόρμες. Το “hello world” prototype θα είναι ένα wifi-θερμόμετρο. Στόχος σε αυτή τη φάση είναι η αξιολόγηση και σύγκριση των πλεονεκτημάτων/μειονεκτημάτων της κάθε πλατφόρμας και η καταγραφή των διαφόρων αποτελεσμάτων (π.χ. κατανάλωση) ώστε να επιλέξουμε με ποια θα προχωρήσουμε. 

**2. Κατασκευή πρωτότυπου hardware**
Θα προχωρήσουμε στην πλήρη υλοποίηση των προδιαγραφών του μετεωρολογικού μας σταθμού, προσθέτοντας όλα τα απαραίτητα περιφερειακά εξαρτήματα και αισθητήρες, πανω στη βελτιστη IoT πλατφόρμα (αποτέλεσμα προηγουμένης φάσης). Θα αναπτύξουμε το firmware  (arduino compatible) που θα διαχειρίζεται ολα τα δεδομενα, και θα στέλνει περιοδικά σε κάποιο “ανοιχτό/free” cloud service όλα τα μετεωρολογικά δεδομένα (π.χ. https://thingspeak.com/) 

**3. Σύνδεση hardware με backend ανοιχτό λογισμικό με δυνατότητα διαχείρισης big data**
Μέσα από προηγούμενες εμπειρίες έχουμε καταλήξει σε μια σειρά από προτεινόμενες τεχνολογίες για την διαχείριση big data οπως: Spring framework, Apache HBASE, Spark, mongoDB, Solr, etc.  οι οποίες όμως είναι αρκετά συνθέτες στη λειτουργία/εγκατάσταση. Στο opensource (CPAL-1.0) project sitewhere.org, υπάρχει ήδη μια πολύ καλή διασύνδεση/integration αυτών των τεχνολογιών, οπότε θα κάνουμε τις απαραίτητες τροποποιήσεις στο firmware και στο sitewhere server, ώστε να γίνει η σύνδεση του hardware μας με αυτό το backend IoT server και θα αξιοποιήσουμε τα μετεωρολογικά δεδομένα με διάφορους τρόπους, π.χ. απεικόνιση σε διαδραστικό χάρτη σε πραγματικό χρόνο, γραφήματα με ιστορικά δεδομένα, κτλ. Θα χρησιμοποιήσουμε MQTT ή HTTP πρωτόκολλο για να διευκολύνουμε και μελλοντικές διασυνδέσεις με τρίτα συστήματα ή/και άλλο IoT hardware.

**4. Μελέτη custom PCB και πρωτότυπου 3D printed πλαισίου (enclosure)**
θα εκπονήσουμε μελέτη για να εκτιμήσουμε την διάσταση/layout custom πλακέτας PCB για το hardware. Έχοντας τις τελικές διαστάσεις θα προχωρήσουμε στο σχεδιασμό custom 3d printed ABS plastic enclosure, που να μπορεί να φιλοξενήσει τους εξωτερικούς/εσωτερικούς αισθητήρες, το φωτοβολταϊκό, και τα υπόλοιπα ηλεκτρονικά, μπαταρίες κτλ.


##Άδεια χρήσης##
Τα παραδοτέα θα συμμορφώνονται με το Open Source Hardware Definition v1.0 με άδεια Creative Commons - Attribution - ShareAlike 3.0  για τα σχέδια, κυκλώματα, τεκμηρίωση, κωδικά firmware κτλ 