import org.junit.Test;
import org.junit.Before;
import org.junit.After;
import static org.junit.Assert.*;
import static org.hamcrest.CoreMatchers.is;
import static org.hamcrest.core.IsNot.not;
import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.edge.EdgeDriver;
import org.openqa.selenium.Dimension;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.ui.ExpectedConditions;
import org.openqa.selenium.support.ui.WebDriverWait;
import org.openqa.selenium.JavascriptExecutor;
import java.time.Duration;
import java.util.*;

public class CS_test_2 {
  private WebDriver driver;
  private Map<String, Object> vars;
  JavascriptExecutor js;
  
  @Before
  public void setUp() {
    // 指定EdgeDriver的路径
    System.setProperty("webdriver.edge.driver", "C:\\Users\\lvpei\\Desktop\\web_test\\mycode2025_java_webtest\\drivers\\msedgedriver.exe");
    driver = new EdgeDriver();
    js = (JavascriptExecutor) driver;
    vars = new HashMap<String, Object>();
  }
  
  @After
  public void tearDown() {
    if (driver != null) {
      driver.quit();
    }
  }
  
  @Test
  public void CS_test_1() {
    // 访问一个简单的测试网站
    driver.get("https://webdriveruniversity.com/");
    
    // 设置窗口大小
    driver.manage().window().setSize(new Dimension(1200, 800));
    
    // 等待页面加载并验证标题
    WebDriverWait wait = new WebDriverWait(driver, Duration.ofSeconds(10));
    wait.until(ExpectedConditions.titleContains("WebDriver"));
    
    // 验证页面标题不为空
    assertNotNull("页面标题不应为空", driver.getTitle());
    assertFalse("页面标题不应为空", driver.getTitle().isEmpty());
    
    // 验证页面包含特定元素
    List<WebElement> headings = driver.findElements(By.tagName("h1"));
    assertTrue("页面应包含至少一个h1元素", headings.size() > 0);
  }
}