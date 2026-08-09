import eslint from "@eslint/js";
import sonarjs from "eslint-plugin-sonarjs";
import unicorn from "eslint-plugin-unicorn";

export default [
  {
    ignores: ["build/**", "node_modules/**"],
    linterOptions: {
      reportUnusedDisableDirectives: "error",
      reportUnusedInlineConfigs: "error",
    },
  },
  eslint.configs.recommended,
  sonarjs.configs.recommended,
  unicorn.configs.unopinionated,
  {
    files: ["**/*.js"],
    languageOptions: {
      ecmaVersion: "latest",
      sourceType: "script",
    },
    rules: {
      // QJSEngine reads this descriptor after evaluating each generator file.
      "no-unused-vars": [
        "error",
        {
          argsIgnorePattern: "^(inputs|settings|size)$",
          varsIgnorePattern: "^generator$",
        },
      ],

      // Generator algorithms remain linear so users can follow and customise
      // them without navigating between helper functions.
      "sonarjs/cognitive-complexity": "off",

      // Explicit control flow and short pixel/channel names make the bundled
      // generators easier to follow for users learning from and customising them.
      "unicorn/no-declarations-before-early-exit": "off",
      "unicorn/no-for-loop": "off",
      "unicorn/prefer-minimal-ternary": "off",
      "unicorn/prefer-switch": "off",
      "unicorn/prefer-ternary": "off",
      "unicorn/prevent-abbreviations": "off",
    },
  },
];
