import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintStream;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.stream.Stream;

public class word_converter {

    public static void main(String[] args) throws IOException {
        String in = "../dictionary/words.txt";
        String out = "../dictionary/converted.txt";

        PrintStream fw = new PrintStream( new FileOutputStream(out, false));

        try (Stream<String> stream = Files.lines(Paths.get(in))) {
            stream.forEach(l -> {
                fw.println(l.hashCode());
            });
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            fw.close();
        }
    }
}
