/*=========================================================================
//程序名称:     f_caai_getprice
//隶属子系统:   CA
//产品名称:     BM2PES
//创建人员:     ZHOULI
//创建时间:     2012-11-26
//修改人员:
//修改日期:
//=========================================================================*/
//框架公用头文件，勿删
#include "stdafx.h"
BM2_FUNCTION_EXPORT
double f_caai_getprice(const CString& mat_code, const CString& price_terms, const CString& dept_code, const CString& stats_period, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);

	int i = 0;
	int doFlag = 0;

	CString sqlstr = "";
	CString v_mat_code = "";
	CString v_stats_period = "";
	CString v_dept_code = "";
	CString v_price_terms = "1";
	double v_price = 0;

	// 创建电文处理对象
	CDbCommand cmd_inq(conn);

	try
	{
		v_mat_code = mat_code;
		v_stats_period = stats_period;
		v_price_terms = price_terms;
		v_dept_code = dept_code;
		EDLog(1, 1, "函数完成情况v_mat_code	  			= [%s]", (const char*)v_mat_code);
		//EDLog(1, 1, "函数完成情况v_price_terms	  			= [%s]",(const char*)v_price_terms);	

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	    // DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	        // Oracle 数据库
		default: // 通用
			sqlstr = "SELECT PRICE_UNIT FROM TCAAC12  WHERE rownum=1 "
				" AND PRICE_TERMS = @price_terms "
				" AND MAT_CODE = @mat_code "
				" and VALID_TIME_START<=@stats_period"
				;
			//if (dept_code.Trim() != "")
			//{
			//	sqlstr = sqlstr + " and dept_code=@dept_code ";
			//}
			sqlstr = sqlstr +" ORDER BY  VALID_TIME_START DESC"
				;
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("mat_code", v_mat_code);
		cmd_inq.Parameters.Set("dept_code", v_dept_code);
		cmd_inq.Parameters.Set("price_terms", v_price_terms);
		cmd_inq.Parameters.Set("stats_period", v_stats_period);
		cmd_inq.ExecuteReader();
		if (cmd_inq.Read())
		{
			cmd_inq.Get(1, v_price);
		}
		cmd_inq.Close();
		
		return v_price;
	}
	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CString str = sqlstr + "\r\n" + ex.GetMsg();
		strncpy(s.msg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;
}
