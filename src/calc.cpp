#include "calc.h"

#include <cctype> // for std::isspace
#include <cmath> // various math functions
#include <iostream> // for error reporting via std::cerr

namespace {

const std::size_t max_decimal_digits = 10;
const double eps = 1e-10;
const double pi = 3.1415926535897932384626433;
const double inf = 16331239353195370;

enum class Op {
  ERR, SET, ADD, SUB, MUL, DIV, REM, NEG, POW, SQRT, SIN, COS, RAD, DEG, TAN, CTN, ASIN, ACOS, ATAN, ACTN
};

std::size_t arity(const Op op) {
  switch (op) {
	// error
	case Op::ERR: return 0;
	case Op::RAD: return 0;
	case Op::DEG: return 0;
	  // unary
	case Op::SIN: return 1;
	case Op::COS: return 1;
	case Op::TAN: return 1;
	case Op::CTN: return 1;
	case Op::ASIN: return 1;
	case Op::ACOS: return 1;
	case Op::ATAN: return 1;
	case Op::ACTN: return 1;
	case Op::NEG: return 1;
	case Op::SQRT: return 1;
	  // binary
	case Op::SET: return 2;
	case Op::ADD: return 2;
	case Op::SUB: return 2;
	case Op::MUL: return 2;
	case Op::DIV: return 2;
	case Op::REM: return 2;
	case Op::POW: return 2;
  }
  return 0;
}
Op parse_op(const std::string &line, std::size_t &i) {
  const auto rollback = [&i, &line](const std::size_t n) {
	i -= n;
	std::cerr << "Unknown operation " << line << std::endl;
	return Op::ERR;
  };
  switch (line[i++]) {
	case '0': [[fallthrough]];
	case '1': [[fallthrough]];
	case '2': [[fallthrough]];
	case '3': [[fallthrough]];
	case '4': [[fallthrough]];
	case '5': [[fallthrough]];
	case '6': [[fallthrough]];
	case '7': [[fallthrough]];
	case '8': [[fallthrough]];
	case '9':--i; // a first digit is a part of op's argument
	  return Op::SET;
	case '+': return Op::ADD;
	case '-': return Op::SUB;
	case '*': return Op::MUL;
	case '/': return Op::DIV;
	case '%': return Op::REM;
	case '_': return Op::NEG;
	case '^': return Op::POW;
	case 'A':
	  switch (line[i++]) {
		case 'C':
		  switch (line[i++]) {
			case 'O':
			  switch (line[i++]) {
				case 'S': return Op::ACOS;
				default: return rollback(4);
			  }
			case 'T':
			  switch (line[i++]) {
				case 'N': return Op::ACTN;
				default: return rollback(4);
			  }
			default: return rollback(3);
		  }
		case 'S':
		  switch (line[i++]) {
			case 'I':
			  switch (line[i++]) {
				case 'N': return Op::ASIN;
				default: return rollback(4);
			  }
			default: return rollback(3);
		  }
		case 'T':
		  switch (line[i++]) {
			case 'A':
			  switch (line[i++]) {
				case 'N': return Op::ATAN;
				default: return rollback(4);
			  }
			default: return rollback(3);
		  }
		default: return rollback(2);
	  }
	case 'S':
	  switch (line[i++]) {
		case 'Q':
		  switch (line[i++]) {
			case 'R':
			  switch (line[i++]) {
				case 'T': return Op::SQRT;
				default: return rollback(4);
			  }
			default: return rollback(3);
		  }
		case 'I':
		  switch (line[i++]) {
			case 'N': return Op::SIN;
			default: return rollback(3);
		  }
		default: return rollback(2);
	  }
	case 'C':
	  switch (line[i++]) {
		case 'O':
		  switch (line[i++]) {
			case 'S': return Op::COS;
			default: return rollback(3);
		  }
		case 'T':
		  switch (line[i++]) {
			case 'N': return Op::CTN;
			default: return rollback(3);
		  }
		default: return rollback(2);
	  }
	case 'R':
	  switch (line[i++]) {
		case 'A':
		  switch (line[i++]) {
			case 'D': return Op::RAD;
			default: return rollback(3);
		  }
		default: return rollback(2);
	  }
	case 'D':
	  switch (line[i++]) {
		case 'E':
		  switch (line[i++]) {
			case 'G': return Op::DEG;
			default: return rollback(3);
		  }
		default: return rollback(2);
	  }
	case 'T':
	  switch (line[i++]) {
		case 'A':
		  switch (line[i++]) {
			case 'N': return Op::TAN;
			default: return rollback(3);
		  }
		default: return rollback(2);
	  }
	default: return rollback(1);
  }
}

std::size_t skip_ws(const std::string &line, std::size_t i) {
  while (i < line.size() && std::isspace(line[i])) {
	++i;
  }
  return i;
}

double parse_arg(const std::string &line, std::size_t &i) {
  double res = 0;
  std::size_t count = 0;
  bool good = true;
  bool integer = true;
  double fraction = 1;
  while (good && i < line.size() && count < max_decimal_digits) {
	switch (line[i]) {
	  case '0': [[fallthrough]];
	  case '1': [[fallthrough]];
	  case '2': [[fallthrough]];
	  case '3': [[fallthrough]];
	  case '4': [[fallthrough]];
	  case '5': [[fallthrough]];
	  case '6': [[fallthrough]];
	  case '7': [[fallthrough]];
	  case '8': [[fallthrough]];
	  case '9':
		if (integer) {
		  res *= 10;
		  res += line[i] - '0';
		} else {
		  fraction /= 10;
		  res += (line[i] - '0') * fraction;
		}
		++i;
		++count;
		break;
	  case '.':integer = false;
		++i;
		break;
	  default:good = false;
		break;
	}
  }
  if (i < line.size()) {
	std::cerr << "Argument isn't fully parsed, suffix left: '" << line.substr(i) << "'" << std::endl;
  }
  return res;
}

double ctn(double current) {
  return 1 / tan(current);
}

double actn(double current) {
  double angle = atan(1 / current);
  if (angle < 0) {
    angle += pi;
  }
  return angle;
}

double to_radians(const double angle, const bool rad_on) {
  return rad_on ? angle : (angle / 180 * pi);
}

double to_degrees(const double angle, const bool rad_on) {
  return rad_on ? angle : angle * 180 / pi;
}

double unary(double current, const Op op, const bool rad_on) {
  switch (op) {
	case Op::NEG: return -current;
	case Op::SQRT:
	  if (current > 0) {
		return std::sqrt(current);
	  } else {
		std::cerr << "Bad argument for SQRT: " << current << std::endl;
		return current;
	  }
	case Op::SIN: return sin(to_radians(current, rad_on));
	case Op::COS: return cos(to_radians(current, rad_on));
	case Op::TAN: {
	  if (std::abs(cos(to_radians(current, rad_on))) > eps) {
		return tan(to_radians(current, rad_on));
	  } else {
	    return inf;
	  }
	}
	case Op::CTN: {
	  if (std::abs(sin(to_radians(current, rad_on))) > eps) {
		return ctn(to_radians(current, rad_on));
	  } else {
		return INFINITY;
	  }
	}
	case Op::ASIN: {
	  if (std::abs(current) <= 1) {
		return to_degrees(asin(current), rad_on);
	  } else {
		std::cerr << "Bad argument for ASIN: " << current << std::endl;
		return INFINITY;
	  }
	}
	case Op::ACOS: {
	  if (std::abs(current) <= 1) {
		return to_degrees(acos(current), rad_on);
	  } else {
		std::cerr << "Bad argument for ACOS: " << current << std::endl;
		return current;
	  }
	}
	case Op::ATAN: {
	  if (std::abs(current) < pi / 2) {
		return to_degrees(atan(current), rad_on);
	  } else {
		std::cerr << "Bad argument for ATAN: " << current << std::endl;
		return current;
	  }
	}
	case Op::ACTN: {
	  if (0 < std::abs(current) && std::abs(current) < pi) {
		return to_degrees(actn(current), rad_on);
	  } else {
		std::cerr << "Bad argument for ACTN: " << current << std::endl;
		return current;
	  }
	}
	default: return current;
  }
}

double binary(const Op op, const double left, const double right) {
  switch (op) {
	case Op::SET:return right;
	case Op::ADD:return left + right;
	case Op::SUB:return left - right;
	case Op::MUL:return left * right;
	case Op::DIV:
	  if (right != 0) {
		return left / right;
	  } else {
		std::cerr << "Bad right argument for division: " << right << std::endl;
		return left;
	  }
	case Op::REM:
	  if (right != 0) {
		return std::remainder(left, right);
	  } else {
		std::cerr << "Bad right argument for remainder: " << right << std::endl;
		return left;
	  }
	case Op::POW:return std::pow(left, right);
	default:return left;
  }
}

} // anonymous namespace

double process_line(const double current, bool &rad_on, const std::string &line) {
  std::size_t i = 0;
  const auto op = parse_op(line, i);

  if (op == Op::RAD) {
    rad_on = true;
    return current;
  }
  if (op == Op::DEG) {
  	rad_on = false;
  	return current;
  }
  switch (arity(op)) {
    case 1: {
	  return unary(current, op, rad_on);
	}
	case 2: {
	  i = skip_ws(line, i);
	  const auto arg = parse_arg(line, i);
	  return binary(op, current, arg);
	}
	default: {
	  break;
	}
  }
  return current;
}
