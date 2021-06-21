"""
Translation module test
"""
import lang

if __name__ == "__main__":
    test = language.Translation(
        eng="Testing this bad boy",
        rus="Что-ж, тестируем!",
    )
    language.set_language("rus")
    print(test)
    language.set_language("eng")
    print(test)
    language.set_language("aggsdwer")
    print(test)
